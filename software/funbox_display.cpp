#include "daisysp.h"
#include "daisy_seed.h"
#include "daisy_petal.h"
#include "funbox.h"
#include "funbox_display.h"

using namespace funbox;

int switchPins[] = {
    Funbox::SWITCH_1_LEFT,
    Funbox::SWITCH_1_RIGHT,
    Funbox::SWITCH_2_LEFT,
    Funbox::SWITCH_2_RIGHT,
    Funbox::SWITCH_3_LEFT,
    Funbox::SWITCH_3_RIGHT
};

bool changedWithTolerance(float old_value, float new_value)
{
    float tolerance = 0.005;
    if (new_value > (old_value + tolerance) || new_value < (old_value - tolerance)) {
        return true;
    } else {
        return false;
    }
}

void Display::InternalInit() {
    fullScreenSize = hwDisplay.GetBounds();
    needsUpdate = true;
    showingDetail = false;
    knobValuesKnownCounter = 10;
    switchValuesKnownCounter = 10;
    showKnobSlider = -1;

    // Consider making this dynamically configurable, but note that it 
    // may cause some planets (e.g. Venus) to fail to build
#if UART_OUT == 1
    petal->seed.StartLog(false);
#endif
}

void Display::InitDisplayI2C(DaisyPetal &petal, Pin scl, Pin sda) {
    this->petal = &petal;

    // Configure the Display
    MyOledDisplay::Config disp_cfg;

    I2CHandle::Config& i2c_conf = disp_cfg.driver_config.transport_config.i2c_config;
    i2c_conf.pin_config.sda = sda;
    i2c_conf.pin_config.scl = scl;

    // XXX should perhaps adjust the I2C peripheral chosen, if not using the standard pins.
    // Or alert/assert here somehow if they're unexpected.

    // Default addr for my display is 0x3c, adjust if necessary
    disp_cfg.driver_config.transport_config.i2c_address = 0x3c;

    hwDisplay.Init(disp_cfg);
    InternalInit();
}

FontDef Display::GetMedFont(const Rectangle* availableSpace)
{
    if (availableSpace == NULL)
        availableSpace = &fullScreenSize;

    if(availableSpace->GetHeight() < 10)
        return Font_6x8;
    else if(availableSpace->GetHeight() < 18)
        return Font_7x10;
    else
        return Font_11x18;
}

FontDef Display::GetLargeFont(const Rectangle* availableSpace)
{
    if (availableSpace == NULL)
        availableSpace = &fullScreenSize;

    if(availableSpace->GetHeight() < 10)
        return Font_6x8;
    if(availableSpace->GetHeight() < 20)
        return Font_7x10;
    else if(availableSpace->GetHeight() < 30)
        return Font_11x18;
    else
        return Font_16x26;
}

// Map the 0-1 raw value into the adjusted value based on knobInfo
float Display::GetAdjustedKnobValue(int knobNum, float rawVal) {
    KnobInfo &info = knobInfo[knobNum-1];
    return info.minVal + (rawVal * (info.maxVal - info.minVal));
}

void Display::FormatFloat(FixedCapStr<10> &dest, float val, int knobNum)
{
    KnobInfo &info = knobInfo[knobNum-1];

    // Map to the defined range for this knob
    val = GetAdjustedKnobValue(knobNum, val);

    // Format to X decimal places, auto-sign.
    dest.AppendFloat(val, info.numDecimals, false, false);
    if (info.unit != NULL) {
        dest.Append(" ");
        dest.Append(info.unit);
    }
};

// Fill whole display with either on pixels or off.
void Display::Fill(bool pixel_on)
{
    hwDisplay.Fill(pixel_on);
}

// Flush current drawn frame to the physical display
void Display::Update()
{
    hwDisplay.Update();
}

// Set the default text to be shown, in large font
void Display::SetBanner(const char* msg)
{
    banner.Reset(msg);
    needsUpdate = true;
}

// Call frequently to allow the display to process timed effects
void Display::Process()
{
    bool made_changes = false;

    // Potentially time out old detail messages
    if (lastDetailDisplayTime != 0) {
        if (System::GetNow() > lastDetailDisplayTime + detailAutoClearDuration) {
            ClearDetail();
            showKnobSlider = -1;
            needsUpdate = true;
        }
    }
        
    if (needsUpdate) {
        hwDisplay.Fill(false); // Clear screen
        if (showingDetail) {
            hwDisplay.WriteStringAligned(detailTop.Cstr(), GetMedFont(), fullScreenSize, Alignment::topCentered, true);
            hwDisplay.WriteStringAligned(detailMiddle.Cstr(), GetMedFont(), fullScreenSize, Alignment::centered, true);
            hwDisplay.WriteStringAligned(detailBottom.Cstr(), GetMedFont(), fullScreenSize, Alignment::bottomCentered, true);
        } else {
            // Default to show the banner
            hwDisplay.WriteStringAligned(banner.Cstr(), GetLargeFont(), fullScreenSize, Alignment::centered, true);
        }
        if (showKnobSlider != -1) {
            DrawSlider(showKnobSlider);
        }
        made_changes = true;
    }

    if (made_changes)
        Update();
}

void Display::CheckKnobMovement(const float* values)
{
    int knobNum = -1;

    if (knobValuesKnownCounter-- > 0) {
        // Initialise the values the first few times we're called - this allows
        // for a few Process() loops before we assume that the values have been properly
        // initialised - usually by the AudioCallback.
        for (int ii = 0; ii < numKnobs; ii++) {
            displayedKnobValues[ii] = values[ii];
        }
    } else {
        for (int ii = 0; ii < numKnobs; ii++)
        {
            if (changedWithTolerance(displayedKnobValues[ii], values[ii]))
            {
                knobNum = ii;    
                displayedKnobValues[ii] = values[ii];
            }
        }    
    }

    if (knobNum != -1)
    {
        FixedCapStr<10> logStr;
        FormatFloat(logStr, values[knobNum], knobNum+1);
        SetDetail("", knobNames[knobNum], logStr.Cstr());

        // And prepare to draw a graphic of the slider position
        showKnobSlider = knobNum;

#if UART_OUT == 1
        // Write to UART as well
        petal->seed.Print(knobNames[knobNum]);
        petal->seed.Print("=");
        petal->seed.PrintLine(logStr.Cstr());
#endif
    }
}

void Display::SetDetail(const char* topmsg, const char *middlemsg, const char *bottommsg)
{
    detailTop.Reset(topmsg);
    detailMiddle.Reset(middlemsg);
    detailBottom.Reset(bottommsg);

    if (detailAutoClearDuration != 0) {
        lastDetailDisplayTime = System::GetNow();
    }

    needsUpdate = true;
    showingDetail = true;
}

void Display::SetDetailAutoClearDuration(uint32_t clearTimeMillis)
{
    detailAutoClearDuration = clearTimeMillis;
}

void Display::ClearDetail()
{
    showingDetail = false;
    needsUpdate = true;
    lastDetailDisplayTime = 0;
}

void Display::IndicateSaving()
{
    SetDetail("", "Saving...", "");
}

void Display::SetSwitchNames(const char** names, const char** valueNames)
{
    switchNames = names;
    switchValueNames = valueNames;
}

void Display::CheckSwitchChanges()
{
    if (switchValuesKnownCounter-- > 0) {
        // Read initial values for later comparison
        for (int ii = 0; ii < 6; ii++) {
            displayedSwitchValues[ii] = petal->switches[switchPins[ii]].Pressed();
        }
    } else {
        int changed_switch = -1;
        bool changed_value_now = false;
        for (int ii = 0; ii < 6; ii++) {
            bool pressedNow = petal->switches[switchPins[ii]].Pressed();
            if (pressedNow != displayedSwitchValues[ii]) {
#if UART_OUT == 1
                petal->seed.PrintLine("sw %d changed", ii);
#endif
                changed_switch = ii;
                changed_value_now = pressedNow;
            }

            if (changed_switch != -1) {
                // We detected a change.  Calculate the details to display.
                int switch_number = changed_switch / 2;
                const char *switchName = switchNames[switch_number];

                // We want value name 0 if value_now is 1 and changed_switch is even.
                // Or value name 2 if value_now is 1 and changed_switch is odd
                // Else value name 1
                int value_number = 1;
                if (changed_value_now) {
                    if (changed_switch % 2 == 0) {
                        value_number = 0;
                    } else {
                        value_number = 2;
                    }
                }
                const char *valueName = switchValueNames[(switch_number * 3) + value_number];

                SetDetail("", switchName, valueName);
            }


            displayedSwitchValues[ii] = pressedNow;
        }
    }
}

void Display::SetKnobNames(int numKnobs, const char** knobNames)
{
    this->numKnobs = numKnobs;
    this->knobNames = knobNames;
}

void Display::SetKnobRange(int knobNum, float minVal, float maxVal, int numDecimals, const char* unit)
{
    KnobInfo &info = knobInfo[knobNum-1];

    info.minVal = minVal;
    info.maxVal = maxVal;
    info.numDecimals = numDecimals;
    info.unit = unit;
}

void Display::DrawSlider(int knobNum)
{
    float currentValue = displayedKnobValues[knobNum];
    KnobInfo &info = knobInfo[knobNum];

    int lineY = (fullScreenSize.GetHeight() / 5.0f);
    int lineStartX = fullScreenSize.GetX();
    int lineEndX = fullScreenSize.GetRight();

    int endMargin = 2;

    // calculate the indicator location
    // This is scaled for the standard 0 - 1 range of the raw knob value
    int indicatorLocationX = lineStartX + endMargin + (currentValue * (fullScreenSize.GetWidth() - (endMargin*2)));

    // Draws a line and a ball where the value is currently located between L/R side
    hwDisplay.DrawLine(lineStartX + endMargin, lineY, lineEndX - endMargin, lineY, true);
    hwDisplay.DrawCircle(indicatorLocationX + endMargin, lineY, 3, true);
    // hwDisplay.SetCursor(lineStartX, lineY - 4);
    // hwDisplay.WriteChar('L', Font_6x8, true);
    // hwDisplay.SetCursor(lineEndX - 6, lineY - 4);
    // hwDisplay.WriteChar('R', Font_6x8, true);
    hwDisplay.DrawLine(lineStartX + endMargin, lineY+2, lineStartX + endMargin, lineY-2, true);
    hwDisplay.DrawLine(lineEndX - endMargin, lineY+2, lineEndX - endMargin, lineY-2, true);

    // If the cooked knob range spans 0, draw a zero marker.
    if ((info.minVal < 0.0f) && (info.maxVal > 0.0f)) {
        int zeroPos = lineStartX + (endMargin*2) + ((-info.minVal / (info.maxVal - info.minVal)) * (fullScreenSize.GetWidth() - (endMargin * 2)));
        hwDisplay.DrawLine(zeroPos, lineY+2, zeroPos, lineY-2, true);
    }
}