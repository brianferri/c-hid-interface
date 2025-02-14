#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi.h>
#include <json.h>
#include <unistd.h>

#define CONFIG_FILE "config.json"

void print_devices(unsigned short vendor_id, unsigned short product_id)
{
	struct hid_device_info *devs, *cur_dev;
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev)
	{
		printf("Device Found      %s\n", cur_dev->path);
		printf("  VID:            %04hx\n", cur_dev->vendor_id);
		printf("  PID:            %04hx\n", cur_dev->product_id);
		printf("  serial_number:  %ls\n", cur_dev->serial_number);
		printf("  Release:        %hx\n", cur_dev->release_number);
		printf("  Manufacturer:   %ls\n", cur_dev->manufacturer_string);
		printf("  Product:        %ls\n", cur_dev->product_string);
		printf("  Usage Page:     %d\n", cur_dev->usage_page);
		printf("  Usage:          %d\n", cur_dev->usage);
		printf("  Interface:      %d\n", cur_dev->interface_number);
		printf("  Bus Type:       %d\n", cur_dev->bus_type);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

void prompt_device_name(const char *prompt, char *device_path)
{
	hid_device *device;
	struct hid_device_info *devs, *cur_dev;
	while (1)
	{
		// 03f0 0c8e
		devs = hid_enumerate(0x03f0, 0x0c8e);
		if (!devs)
		{
			fprintf(stderr, "No devices found.\n");
			sleep(1);
			continue;
		}
		cur_dev = devs;
		while (cur_dev)
		{
			device = hid_open_path(cur_dev->path);
			printf("Opened device: %s\n", cur_dev->path);
			if (device)
			{
				printf("Opened device: %s\n", cur_dev->path);
				unsigned char buf[256];
				memset(buf, 0, sizeof(buf));
				hid_read(device, buf, sizeof(buf));
				if (buf[0] == 0x0a)
				{
					strcpy(device_path, cur_dev->path);
					hid_close(device);
					printf("Device for %s found: %s\n", prompt, device_path);
				}
			}
			else
			{
				fprintf(stderr, "Unable to open device: %ls\n", hid_error(device));
			}
			cur_dev = cur_dev->next;
		}
		hid_free_enumeration(devs);
		sleep(1);
	}
}

void create_config()
{
	char device_path[256];

	prompt_device_name("device", device_path);

	struct json_object *config = json_object_new_object();
	json_object_object_add(config, "device", json_object_new_string(device_path));

	FILE *fp = fopen(CONFIG_FILE, "w");
	fprintf(fp, "%s\n", json_object_to_json_string_ext(config, JSON_C_TO_STRING_PRETTY));
	fclose(fp);
}

struct json_object *load_config()
{
	FILE *fp = fopen(CONFIG_FILE, "r");
	if (!fp)
	{
		create_config();
		fp = fopen(CONFIG_FILE, "r");
		if (!fp)
		{
			perror("Unable to open config file");
			exit(EXIT_FAILURE);
		}
	}

	char buffer[1024];
	fgets(buffer, sizeof(buffer), fp);
	fclose(fp);

	return json_tokener_parse(buffer);
}

int main()
{
	if (geteuid() != 0)
	{
		fprintf(stderr, "This program needs to be run as root.\n");
		exit(EXIT_FAILURE);
	}

	if (hid_init())
	{
		printf("Failed to initialize HIDAPI library.\n");
		exit(EXIT_FAILURE);
	}
	print_devices(0x03f0, 0x0c8e);
	struct json_object *config = load_config();
	const char *device_path = json_object_get_string(json_object_object_get(config, "story_side"));

	printf("Reading input from device: %s\n", device_path);

	hid_device *device = hid_open_path(device_path);

	if (!device)
	{
		fprintf(stderr, "Unable to open one of the devices.\n");
		exit(EXIT_FAILURE);
	}

	unsigned char buf[256];
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		hid_read(device, buf, sizeof(buf));
		printf("Read from device: %s\n", buf);
	}

	hid_close(device);
	hid_exit();

	return 0;
}