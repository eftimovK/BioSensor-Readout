/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    BeepTest.c
 * @brief:   Beeper Driver Test for ADuCM350
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

#include <time.h>
#include <stddef.h>  // for 'NULL'
#include <stdio.h>   // for scanf
#include <string.h>  // for strncmp

#include "beep.h"
#include "gpio.h"

#include "test_common.h"


/* prototypes */
extern int32_t adi_initpinmux (void);
       void    beep_Init      (void);
       void    beep_Start     (void);
       void    beepCallback   (void *hDevice, uint32_t Event, void *pArg);


/* Device handle */
ADI_BEEP_HANDLE hBeep = NULL;
volatile uint8_t nextToneIndex = 0;


/* playlist typedef */
typedef struct {
	ADI_BEEP_NOTE_FREQUENCY_TYPE tone;
	ADI_BEEP_NOTE_DURATION_TYPE  duration;
} TONE_TYPE;

TONE_TYPE playList[] = {

#if 0
	/* Rick Roll (too fast...) */

	/* measure 0 */
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 1 */
	{ ADI_BEEP_FREQ_A5,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_A5,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_3_8  },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 2 */
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_E4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_8    },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 3 */
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_8    },
	{ ADI_BEEP_FREQ_E4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_8    },

	/* measure 4 */
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 5 */
	{ ADI_BEEP_FREQ_A5,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_A5,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_3_8  },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 6 */
	{ ADI_BEEP_FREQ_C5,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_E4,    ADI_BEEP_DUR_8    },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_E4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_8    },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },

	/* measure 7 */
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_8    },
	{ ADI_BEEP_FREQ_E4,    ADI_BEEP_DUR_3_16 },
	{ ADI_BEEP_FREQ_D4,    ADI_BEEP_DUR_16   },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_C4,    ADI_BEEP_DUR_8    },

	/* measure 8 */
	{ ADI_BEEP_FREQ_G4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_F4,    ADI_BEEP_DUR_4    },
	{ ADI_BEEP_FREQ_REST,  ADI_BEEP_DUR_4    },

#else

	/* Twinkle twinkle little star */
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_A4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_A4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_1 },

	/* How I wonder what you are */
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_1 },

    /* Up above the world so high */
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_1 },

    /* Like a diamond in the sky */
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_1 },

    /* Twinkle twinkle little star */
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_A4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_A4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_G4,		ADI_BEEP_DUR_1 },

    /* How I wonder what you are */
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_F4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_E4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_D4,		ADI_BEEP_DUR_2 },
	{ ADI_BEEP_FREQ_C4,		ADI_BEEP_DUR_1 },
#endif
};
uint8_t playLength = sizeof(playList)/sizeof(playList[0]);


int main (void)
{
    bool_t busy = true;

    /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();

    /* initialize the driver */
    beep_Init();

	/* play the tune */
    beep_Start();

    /* wait here until the beeper completes the playlist */
    while (busy) {
        /* update busy flag */
        if (ADI_BEEP_SUCCESS != adi_Beep_GetBusyStatus(hBeep, &busy))
            FAIL("adi_Beep_GetBusyStatus failed");
    }

    /* release driver */
    if (ADI_BEEP_SUCCESS != adi_Beep_UnInit(hBeep)) {
        FAIL("adi_Beep_UnInit failed");
    }

    PASS();
}


void beep_Init (void) {

    /* run the pin mux setup code */
    adi_initpinmux();

    /* initialize the beeper driver */
    if (ADI_BEEP_SUCCESS != adi_Beep_Init(ADI_BEEP_DEVID_0, &hBeep)) {
        FAIL("adi_Beep_Init failed");
    }

    /* register a callback for toneA start interrupts */
    if (ADI_BEEP_SUCCESS != adi_Beep_RegisterCallback (hBeep, beepCallback, ADI_BEEP_INTERRUPT_TONEA_START)) {
        FAIL("adi_beep_RegisterCallback failed");
    }

    /* Program the number of tone A/B pairs in the sequence */
    /* (Note: sequence mode is preferred for multiple tones because it avoids the protracted overhead
        introduced by the controller trying to play a song comprised of multiple pulse mode tones that
        auto-disable each time and must be manually re-enabled to start the next pulse mode tone.) */
     if (ADI_BEEP_SUCCESS != adi_Beep_SetSequence(hBeep, playLength))
        FAIL("adi_Beep_SetSequence failed");

}


void beep_Start (void) {

    /* preset the first toneA content (the rest of the sequence is interrupt-driven) */
     if (ADI_BEEP_SUCCESS != adi_Beep_SetTone(hBeep, ADI_BEEP_TONEA, playList[nextToneIndex].tone, playList[nextToneIndex].duration))
        FAIL("adi_Beep_SetTone failed");

     /* increment the index */
     nextToneIndex++;

    /* Set toneB once to a constant minimal silent tone to "play" between toneA tones (ToneB is required for sequence mode) */
     if (ADI_BEEP_SUCCESS != adi_Beep_SetTone(hBeep, ADI_BEEP_TONEB, ADI_BEEP_FREQ_REST, (ADI_BEEP_NOTE_DURATION_TYPE)1))
        FAIL("adi_Beep_SetTone failed");

    /* start the play */
    if (ADI_BEEP_SUCCESS != adi_Beep_SetDeviceEnable(hBeep, true))
        FAIL("adi_beep_EnableDevice failed");
}


/* Application Callback */
void beepCallback(void *hDevice, uint32_t status, void *pArg /*not used*/) {

    /* service the toneA start interrupts while playlist is incomplete */
    if ((status & ADI_BEEP_INTERRUPT_TONEA_START) && (nextToneIndex < playLength)) {

        /* program next tone... */
        if (ADI_BEEP_SUCCESS != adi_Beep_SetTone(hDevice, ADI_BEEP_TONEA, playList[nextToneIndex].tone, playList[nextToneIndex].duration))
            FAIL("adi_Beep_SetTone failed");

		 /* increment the index */
		 nextToneIndex++;
    }
}
