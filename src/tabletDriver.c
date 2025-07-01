// Custom device:
#include <fcntl.h>
#include <linux/uinput.h>

#include <stdbool.h>

#include "argument_parser.h"
#include "ssh.h"

/* Global variables */
int fd;
bool verbose;
ssh_session session;
static ssh_channel input_channel = NULL;

/* This function only prints if verbose is enabled */
static inline void print_verbose(const char *format, ...) {
    if (verbose) {
        printf(format);
    }
}

void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

/* Passes given input event (received from tablet) to the virtual tablet */
void pass_input_event(struct input_event ie) {
   write(fd, &ie, sizeof(ie));
}

// Helper: Get the pen device path from the remote tablet
void get_pen_device_path(char *pen_device_path, size_t path_len) {
    ssh_channel channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Failed to create SSH channel\n");
        exit(1);
    }
    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Failed to open SSH channel session\n");
        ssh_channel_free(channel);
        exit(1);
    }
    rc = ssh_channel_request_exec(channel, "readlink -f /dev/input/touchscreen0");
    if (rc != SSH_OK) {
        fprintf(stderr, "Failed to exec readlink command\n");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        exit(1);
    }
    int len = ssh_channel_read(channel, pen_device_path, path_len - 1, 0);
    if (len <= 0) {
        fprintf(stderr, "Failed to read pen device path from SSH channel\n");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        exit(1);
    }
    pen_device_path[len] = '\0';
    // Remove trailing newline if present
    char *newline = strchr(pen_device_path, '\n');
    if (newline) *newline = '\0';
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

// Helper: Open a persistent SSH channel and start cat on the device
void open_input_channel(const char *pen_device_path) {
    input_channel = ssh_channel_new(session);
    if (input_channel == NULL) {
        fprintf(stderr, "Failed to create SSH channel\n");
        exit(1);
    }
    int rc = ssh_channel_open_session(input_channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Failed to open SSH channel session\n");
        ssh_channel_free(input_channel);
        exit(1);
    }
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cat %s", pen_device_path);
    print_verbose("Opening persistent input channel: %s\n", cmd);
    rc = ssh_channel_request_exec(input_channel, cmd);
    if (rc != SSH_OK) {
        fprintf(stderr, "Failed to exec remote cat command\n");
        ssh_channel_close(input_channel);
        ssh_channel_free(input_channel);
        exit(1);
    }
}

// Helper: Read a single input_event from the persistent SSH channel
void read_remote_input_event(struct input_event *ie) {
    size_t total = 0;
    char *ptr = (char *)ie;
    while (total < sizeof(struct input_event)) {
        int n = ssh_channel_read(input_channel, ptr + total, sizeof(struct input_event) - total, 0);
        if (n < 0) {
            fprintf(stderr, "Failed to read input_event from SSH channel (error)\n");
            ssh_channel_close(input_channel);
            ssh_channel_free(input_channel);
            exit(1);
        }
        if (n == 0) {
            if (total < sizeof(struct input_event)) {
                fprintf(stderr, "EOF before reading full input_event (%zu/%zu bytes)\n", total, sizeof(struct input_event));
                ssh_channel_close(input_channel);
                ssh_channel_free(input_channel);
                exit(1);
            }
            break;
        }
        total += n;
    }
}

/* Gets the input event from the tablet using SSH */
struct input_event get_input_event() {
    static char pen_device_path[128] = "";
    static int channel_opened = 0;
    struct input_event ie;
    // Only detect the pen device path and open channel once
    if (pen_device_path[0] == '\0') {
        get_pen_device_path(pen_device_path, sizeof(pen_device_path));
    }
    if (!channel_opened) {
        open_input_channel(pen_device_path);
        channel_opened = 1;
    }
    read_remote_input_event(&ie);
    return ie;
}


void addAbsCapability(int fd, int code, int32_t value, int32_t min, int32_t max, int32_t resolution, int32_t fuzz, int32_t flat) {
   ioctl(fd, UI_SET_ABSBIT, code); // Add capability

   struct input_absinfo abs_info;
   abs_info.value = value;
   abs_info.minimum = min;
   abs_info.maximum = max;
   abs_info.resolution = resolution;
   abs_info.fuzz = fuzz;
   abs_info.flat = flat;

   struct uinput_abs_setup abs_setup;
   abs_setup.code = code;
   abs_setup.absinfo = abs_info;

   if(ioctl(fd, UI_ABS_SETUP, &abs_setup) < 0) { // Set abs data
      perror("Failed to absolute info to uinput-device (old kernel?)");
      exit(1);
   }
}

   void closeDevice() {
      ioctl(fd, UI_DEV_DESTROY);
      close(fd);
   }

int main(int argc, char** argv) {
  /*
   * Setup argument parsing
   */
  struct arguments arguments;
  /* Default values. */
  arguments.verbose = 0;
  arguments.private_key_file = "";
  arguments.address = "10.11.99.1";
  arguments.port = 22;
  arguments.orientation = "right";
  arguments.threshold = 600;

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if (arguments.verbose) {
    verbose = true;
    print_arguments(&arguments);
  }

   // Create virtual tablet:
   fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

   ioctl(fd, UI_SET_EVBIT, EV_KEY);
   ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_PEN); // BTN_TOOL_PEN == 1 means that the pen is hovering over the tablet
   ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH); // BTN_TOUCH == 1 means that the pen is touching the tablet
   ioctl(fd, UI_SET_KEYBIT, BTN_STYLUS);  // To satisfy libinput. Is not used.

   // See https://python-evdev.readthedocs.io/en/latest/apidoc.html#evdev.device.AbsInfo.resolution
   // Resolution = max(20967, 15725) / (21*10)  # Height of display is 21cm. Format is units/mm. => ca. 100 (99.84285714285714)
   // Tilt resolution = 12600 / ((math.pi / 180) * 140 (max angle)) (Format: units/radian) => ca. 5074 (5074.769042587292)
   ioctl(fd, UI_SET_EVBIT, EV_ABS);
   addAbsCapability(fd, ABS_PRESSURE, /*Value:*/ 0,     /*Min:*/ 0,     /*Max:*/ 4095,  /*Resolution:*/ 0,   /*Fuzz:*/ 0, /*Flat:*/ 0);
   addAbsCapability(fd, ABS_DISTANCE, /*Value:*/ 95,    /*Min:*/ 0,     /*Max:*/ 255,   /*Resolution:*/ 0,   /*Fuzz:*/ 0, /*Flat:*/ 0);
   addAbsCapability(fd, ABS_TILT_X,   /*Value:*/ 0,     /*Min:*/ -9000, /*Max:*/ 9000, /*Resolution:*/ 5074, /*Fuzz:*/ 0, /*Flat:*/ 0);
   addAbsCapability(fd, ABS_TILT_Y,   /*Value:*/ 0,     /*Min:*/ -9000, /*Max:*/ 9000, /*Resolution:*/ 5074, /*Fuzz:*/ 0, /*Flat:*/ 0);
   addAbsCapability(fd, ABS_X,        /*Value:*/ 11344, /*Min:*/ 0,     /*Max:*/ 20967, /*Resolution:*/ 100, /*Fuzz:*/ 0, /*Flat:*/ 0);
   addAbsCapability(fd, ABS_Y,        /*Value:*/ 10471, /*Min:*/ 0,     /*Max:*/ 15725, /*Resolution:*/ 100, /*Fuzz:*/ 0, /*Flat:*/ 0);

   struct uinput_setup usetup;
   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_VIRTUAL;
   //usetup.id.vendor = 0x1235; /* sample vendor */
   //usetup.id.product = 0x7890; /* sample product */
   usetup.id.version = 0x3;
   strcpy(usetup.name, "reMarkableTablet-FakePen");  // Has to end with "pen" to work in Krita!!!
   if(ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
      perror("Failed to setup uinput-device (old kernel?)");
      return 1;
   }
   
   if(ioctl(fd, UI_DEV_CREATE) < 0) {
      perror("Failed to create uinput-device");
      return 1;
   }

   /* Connect to reMarkable */
  create_ssh_session(&session, arguments.address, &arguments.port);
   printf("Connected\n");

   while(1) {
      /* Get packet and pass it to emit() function */
      //emit(fd, packet.type, packet.code, packet.value);
      struct input_event ie = get_input_event();
      pass_input_event(ie);
   }

   // Close virtual tablet:
   closeDevice();

   /* Cleanup SSH input channel */
   if (input_channel) {
      ssh_channel_send_eof(input_channel);
      ssh_channel_close(input_channel);
      ssh_channel_free(input_channel);
      input_channel = NULL;
   }

   /* Cleanup ssh connection */
   ssh_disconnect(session);
   ssh_free(session);

   return 0;
}
