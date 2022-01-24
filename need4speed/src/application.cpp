#include "glez.h"

int main(int argc, char** argv) 
{
	glez::init();

	glez::scene* scene = new glez::scene();
	delete scene;

	return 0;
}
