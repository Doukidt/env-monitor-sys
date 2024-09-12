#include "esys/esysControl.h"

int main()
{
	ems::esysControl& esys = ems::esysControl::getInstance();
	return esys.sysRun();
}
