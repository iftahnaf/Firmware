/****************************************************************************
 *
 *   Copyright (c) 2018-2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "PMW3901.hpp"

#include <px4_platform_common/getopt.h>

namespace pmw3901
{
PMW3901 *g_dev{nullptr};

static int start(float yaw_rotation_degrees)
{
	if (g_dev != nullptr) {
		PX4_WARN("already started");
		return 0;
	}

	// create the driver
#if defined(PX4_SPI_BUS_EXPANSION) && defined(PX4_SPIDEV_EXPANSION_2) // crazyflie flow deck
	g_dev = new PMW3901(PX4_SPI_BUS_EXPANSION, PX4_SPIDEV_EXPANSION_2, yaw_rotation_degrees);
#elif defined(PX4_SPI_BUS_EXTERNAL1) && defined(PX4_SPIDEV_EXTERNAL1_1) // fmu-v5 ext CS1
	g_dev = new PMW3901(PX4_SPI_BUS_EXTERNAL1, PX4_SPIDEV_EXTERNAL1_1, yaw_rotation_degrees);
#elif defined(PX4_SPI_BUS_EXTERNAL) && defined(PX4_SPIDEV_EXTERNAL) // fmu-v4 extspi
	g_dev = new PMW3901(PX4_SPI_BUS_EXTERNAL, PX4_SPIDEV_EXTERNAL, yaw_rotation_degrees);
#else
	PX4_ERR("External SPI not available");
	return -1;
#endif

	if (g_dev == nullptr) {
		PX4_ERR("alloc failed");
		return -1;
	}

	if (g_dev->init() != PX4_OK) {
		PX4_ERR("driver init failed");
		delete g_dev;
		g_dev = nullptr;
		return -1;
	}

	return 0;
}

static int stop()
{
	if (g_dev == nullptr) {
		PX4_WARN("driver not running");
		return -1;
	}

	delete g_dev;
	g_dev = nullptr;

	return 0;
}

static int status()
{
	if (g_dev != nullptr) {
		g_dev->print_info();
		return 0;
	}

	PX4_WARN("driver not running");
	return -1;
}

static int usage()
{
	PX4_INFO("missing command: try 'start', 'stop', 'status'");
	PX4_INFO("options:");
	PX4_INFO("    -Y yaw rotation (degrees)");

	return 0;
}

} // namespace pmw3901

extern "C" int pmw3901_main(int argc, char *argv[])
{
	float yaw_rotation_degrees = NAN;
	int myoptind = 1;
	int ch = 0;
	const char *myoptarg = nullptr;

	while ((ch = px4_getopt(argc, argv, "Y:", &myoptind, &myoptarg)) != EOF) {
		switch (ch) {
		case 'Y':
			yaw_rotation_degrees = atof(myoptarg);
			break;

		default:
			return pmw3901::usage();
		}
	}

	const char *verb = argv[myoptind];

	if (!strcmp(verb, "start")) {
		return pmw3901::start(yaw_rotation_degrees);

	} else if (!strcmp(verb, "stop")) {
		return pmw3901::stop();

	} else if (!strcmp(verb, "status")) {
		return pmw3901::status();
	}

	return pmw3901::usage();
}
