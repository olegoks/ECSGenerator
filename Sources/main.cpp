
#include <ecs_generator3.hpp>

#include <string>
int main(int argc, char** argv) {

	ASSERT_FMSG(argc > 0, "First parameter must be config file name.");

	using namespace OksEngine;

	const ecs_generator3::CreateInfo engineCreateInfo{
		argc, argv
	};

	ecs_generator3 generator{ engineCreateInfo };

	generator.Run();

	return 0;
}