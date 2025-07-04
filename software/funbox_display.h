#pragma once

#include "daisy.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;

// Choose the appropriate display type by changing this line:
using MyOledDisplay = OledDisplay<SSD130xI2c128x64Driver>;

// Default I2C1 peripheral, edit if necessary
const Pin default_scl = seed::D11;
const Pin default_sda = seed::D12;

// Choose whether to include serial UART output
#define UART_OUT 0

namespace funbox
{
    struct KnobInfo {
        // Default to 0-100%
        float minVal = 0.0f;
        float maxVal = 100.0f;
        int   numDecimals = 0;
        const char *unit = "%";
    };

    class Display
    {
        public:
            Rectangle fullScreenSize;
            MyOledDisplay hwDisplay;
            
            void InitDisplayI2C(DaisyPetal &petal, Pin scl = default_scl, Pin sda = default_sda);
            void Fill(bool pixel_on);
            void Update();
            void Process();  // Call frequently from main loop to allow for automatic timed effects.

            // Detailed display is divided into 3 lines : top, middle, bottom
            // The detailed display will be cleared back to the banner after 2000 milliseconds by
            // default, or use the following method to set a different duration.  0 will disable auto-clear.
            void SetDetailAutoClearDuration(uint32_t clearTimeMillis);
            void ClearDetail();
            void SetDetail(const char* msgTop, const char* msgMiddle, const char* msgBottom);

            // Default display - a short string in large font (usually the planet name)
            void SetBanner(const char* msg);

            // Signal that settings are being saved
            void IndicateSaving();

            // Assign names to the parameter knobs
            void SetKnobNames(int numKnobs, const char** knobNames);
            // Assume all knobs have a range of 0 - 100 % unless set with the following method.
            void SetKnobRange(int knobNum, float minVal, float maxVal, int numDecimals=2, const char* unit=NULL);
            // Check knobs for any changes, and display details of any found.
            void CheckKnobMovement(const float* values);

            // Check switches for changes, and display details of any found
            void SetSwitchNames(const char** switchNames, const char** switchValueNames);
            void CheckSwitchChanges();

            // Utility method to format a float into a string suitable for display
            // Includes mapping and units based on knob info for knobNum
            void FormatFloat(FixedCapStr<10> &dest, float value, int knobNum);

            // Find an appropriate font for the given rectangle.  Mostly used internally, but made
            // available for code that wants to call through to the underlying hwDisplay object.
            // Default rectangle is the full screen size.
            FontDef GetMedFont(const Rectangle* availableSpace = NULL);
            FontDef GetLargeFont(const Rectangle* availableSpace = NULL);

            // Draw a slider representing the current value of a knob
            void DrawSlider(int knobNum);

            // Map raw 0-1 knob value to display value based on knobInfo
            float GetAdjustedKnobValue(int knobNum, float rawVal);


        private:
            int  numKnobs = 0;
            uint32_t lastDetailDisplayTime = 0L;
            uint32_t detailAutoClearDuration = 2000;
            bool showingDetail;
            int knobValuesKnownCounter;
            int switchValuesKnownCounter;

            // true if the display should be forcibly redrawn at the next opportunity
            bool needsUpdate;

            // Set to a knob index to draw a slider image for that knob value when drawing
            // the details view.
            int showKnobSlider;

            FixedCapStr<10> detailTop;
            FixedCapStr<10> detailMiddle;
            FixedCapStr<10> detailBottom;
            FixedCapStr<10> banner;

            void InternalInit();

            float displayedKnobValues[7];
            bool displayedSwitchValues[6];
            const char** switchNames;
            const char** switchValueNames;
            const char** knobNames;

            KnobInfo knobInfo[6];

            DaisyPetal *petal;
    };

}