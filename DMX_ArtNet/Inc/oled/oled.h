#ifndef __OLED_H_
#define __OLED_H_

#include "oled/ssd1306.h"
#include "platform.h"
#include "systimer.h"

// Addr on OLED is already shifted by 1
#define OLED_I2CADDR 0x3C

typedef enum {
    OLEDB_None,
    OLEDB_Back,
    OLEDB_Confirm,
    OLEDB_Up,
    OLEDB_Down
} OLED_Buttons;

typedef enum {
    OLEDM_Offline,
    OLEDM_Sleeping,
    OLEDM_Active,
} OLED_Mode;

typedef enum {
    OLEDE_Text,
    OLEDE_Ip,
    OLEDE_Int,
    OLEDE_List
} OLED_EditMode;

typedef struct {
    void (*OnRender)();
    void (*OnNavigate)(OLED_Buttons btn);
    void (*OnInit)();
    void (*OnConfirm)();

    char ScreenMetadata;
    unsigned char ScreenParameter;

    char **ListItems;
    unsigned short ListSize;
} OLED_ScreenData;

typedef struct {
    OLED_EditMode Mode;

    char ValueCopy[64];
    union {
        char Value[64];
        struct {
            signed char ValueIndex;
            const char **ValueList;
            unsigned char ValueListSize;
        };
    };

    short Minimum;
    short Maximum;
    char FieldLength;
    unsigned char Disabled;
} OLED_EditFieldMetadata;

typedef struct {
    OLED_EditFieldMetadata Fields[10];

    unsigned char Changed;
    signed char CursorPosition;
    union {
    	signed char SelectedValue;
    	char SelectedChar;
    };
} OLED_EditMetadata;

typedef struct {
    OLED_ScreenData *Source;
    short MenuItem;
    OLED_ScreenData *Target;
    char Parameter;
} OLED_Transition;

typedef struct {
    OLED_Mode State;
    OLED_ScreenData *ActiveScreen;
    OLED_EditMetadata EditInfo;

    OLED_Buttons LastButton;

    void (*OnRender)();
    void (*OnNavigate)(OLED_Buttons btn);
    unsigned int OverlayTimeout;

    signed char SelectedMenuItem;
} OLED_State;

void OLED_Init();
void OLED_Tick();
void OLED_ForceRefresh();

#endif
