#include <nuttx/config.h>
#include <unistd.h>
#include "stdio.h"
#include "poll.h"
#include "string.h"

#include "uORB/uORB.h"
#include "uORB/topics/sensor_combined.h"
#include "uORB/topics/vehicle_attitude.h"

_EXPORT int px4_simple_app_main(int argc, char *argv[]);

int px4_simple_app_main(int argc, char *argv[])
{
	printf("hello sky\n");

	//subscribe
	int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));
	orb_set_interval(sensor_sub_fd, 1000);

	//advertise
	struct vehicle_attitude_s att;
	memset(&att, 0, sizeof(att));
	orb_advert_t att_pub = orb_advertise(ORB_ID(vehicle_attitude), &att);

	struct pollfd fds[] =
	{
		/* data */
		{ .fd=sensor_sub_fd, .events = POLLIN},
	};

	int error_counter = 0;

	for (int i = 0; i < 5; i++) {
		int poll_ret = poll(fds, 1, 1000);

		if( poll_ret == 0) {
			printf("[px4_simple_app] Got no data with a second\n");
		} else if (poll_ret < 0) {
			if(error_counter < 10 || error_counter % 50 == 0) {
				printf("[px4_simple_app] ERROR return value from poll(): %d\n", poll_ret);
			}

			error_counter ++;

		} else {
			if (fds[0].revents & POLLIN)
			{
				struct sensor_combined_s raw;

				orb_copy(ORB_ID(sensor_combined), sensor_sub_fd, &raw);
				printf("read accelerometer\n");

				att.roll = raw.accelerometer_m_s2[0];
				att.pitch = raw.accelerometer_m_s2[1];
				att.yaw = raw.accelerometer_m_s2[2];
				orb_publish(ORB_ID(vehicle_attitude), att_pub, &att);
			}
		}
	}

	return 0;
}
