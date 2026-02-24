//
// Created by Anh Huynh on 23.2.2026.
//

#include "SwitchPair.h"

bool SwitchPair::sw0_pressed() const
{
	return sw0.read();
}

bool SwitchPair::sw2_pressed() const
{
	return sw2.read();
}

bool SwitchPair::both_btn_pressed() const
{
	return sw0_pressed() && sw2_pressed();
}



