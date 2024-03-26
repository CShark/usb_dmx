#include "oled/oled.h"
#include "config.h"
#include "dmx.h"
#include "lwip/netif.h"
#include "oled/fonts.h"

static void OLED_InitSendData(const unsigned char *buffer, unsigned short len, void (*callback)());
static void OLED_SendData(const unsigned char *buffer, unsigned short len);

static void OLED_RenderBoot();
static void OLED_RenderDFU();
static void OLED_RenderIpOverview();

static void OLED_NavigateMenu(OLED_Buttons btn);
static void OLED_RenderMenu();
static void OLED_NavigateEdit(OLED_Buttons btn);
static void OLED_RenderEdit();
static void OLED_NavigateValueSelect(OLED_Buttons btn);
static void OLED_RenderValueSelect();
static void OLED_NavigateEditSelect(OLED_Buttons btn);
static void OLED_RenderEditSelect();
static void OLED_NavigateConfirm(OLED_Buttons btn);
static void OLED_RenderConfirm();
static void OLED_RenderInvalidValue();

static void OLED_InitPort();
static void OLED_InitIp();
static void OLED_InitFirmware();

static void OLED_ConfirmPort();
static void OLED_ConfirmIp();
static void OLED_ConfirmReset();

static char OLED_ValidateFieldValue(unsigned char i);

static const unsigned char *pending_buffer;
static unsigned short pending_len;
static void (*pending_callback)();

static unsigned int lastTick;
static unsigned int bootTimer;
static unsigned int lastInputTick;
static unsigned int lastInputRepeatTick;
static unsigned int dfuTimer;
static unsigned int inputRepeatTimer;
static unsigned char forceRefresh = 0;

static char *opt_disabled[] = {"Disabled", "Enabled"};
static char *opt_yesno[] = {"No", "Yes"};
static char *opt_ipMode[] = {"AutoIP", "Static", "DHCP"};
static char *opt_portDir[] = {"Output", "Input"};
static char *opt_portMode[] = {"Cont", "Delta"};
static char *opt_failover[] = {"Hold", "Zero", "Full", "Scene"};

// needs to be separate, otherwise it will hartfault...
static char *listItems[] = {
    // Menu - 8
    "Port 1",
    "Port 2",
    "Port 3",
    "Port 4",
    "IP Config",
    "Firmware Info",
    "Reset Config",
    "Reboot DFU",
    // Ports - 10
    "Name",
    "Description",
    "Direction",
    "Universe",
    "ACN Prio",
    "In Disable",
    "RDM",
    "Port Mode",
    "Failover",
    "Rec Failover",
    // IP Config - 8
    "Mode",
    "Static IP",
    "Netmask",
    "Gateway",
    "DHCP Server",
    "Device IP",
    "Host IP",
    "DHCP Mask",
    // Firmware - 2
    "Version",
    "Firmware Id",
};

static OLED_ScreenData oledScreens[] = {
    {
        .OnRender = OLED_RenderBoot,
        .OnNavigate = NULL,
        .OnInit = NULL,
    },
    {
        .OnRender = OLED_RenderIpOverview,
        .OnNavigate = OLED_NavigateMenu,
        .OnInit = NULL,
    },
    {
        .OnRender = OLED_RenderMenu,
        .OnNavigate = OLED_NavigateMenu,
        .OnInit = NULL,
        .ListItems = &listItems[0],
        .ListSize = 8,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = OLED_NavigateEdit,
        .OnInit = OLED_InitPort,
        .OnConfirm = OLED_ConfirmPort,
        .ListItems = &listItems[8],
        .ListSize = 10,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = OLED_NavigateEdit,
        .OnInit = OLED_InitIp,
        .OnConfirm = OLED_ConfirmIp,
        .ListItems = &listItems[8 + 10],
        .ListSize = 8,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = OLED_NavigateMenu,
        .OnInit = OLED_InitFirmware,
        .ListItems = &listItems[8 + 10 + 8],
        .ListSize = 2,
    },
    {
        .OnRender = OLED_RenderConfirm,
        .OnNavigate = OLED_NavigateConfirm,
        .OnConfirm = OLED_ConfirmReset,
        .OnInit = NULL,
    },
    {
        .OnRender = OLED_RenderDFU,
        .OnNavigate = NULL,
        .OnInit = NULL,
    },
};

static OLED_Transition oledTransitionMap[] = {
    {.Source = &oledScreens[1], .MenuItem = -1, .Target = &oledScreens[2]},

    {.Source = &oledScreens[2], .MenuItem = 0, .Target = &oledScreens[3], .Parameter = 0},
    {.Source = &oledScreens[2], .MenuItem = 1, .Target = &oledScreens[3], .Parameter = 1},
    {.Source = &oledScreens[2], .MenuItem = 2, .Target = &oledScreens[3], .Parameter = 2},
    {.Source = &oledScreens[2], .MenuItem = 3, .Target = &oledScreens[3], .Parameter = 3},
    {.Source = &oledScreens[2], .MenuItem = 4, .Target = &oledScreens[4]},
    {.Source = &oledScreens[2], .MenuItem = 5, .Target = &oledScreens[5]},
    {.Source = &oledScreens[2], .MenuItem = 6, .Target = &oledScreens[6]},
    {.Source = &oledScreens[2], .MenuItem = 7, .Target = &oledScreens[7]},
};

static const unsigned char oledTransitions = 9;

static OLED_State oledState = {.State = OLEDM_Offline, .ActiveScreen = &oledScreens[0], .OnRender = NULL, .OnNavigate = NULL};

void OLED_Init() {
    // Do not enable OLED if subboard is not detected
    if ((GPIOB->IDR & (1 << 15)) == 0) {
        return;
    }

    GPIOB->MODER |= (0x3 << GPIO_MODER_MODE15_Pos);
    oledState.State = OLEDM_Active;

    // Initialize I²C & DMA
    DMAMUX1_Channel8->CCR = 23; // I2C4_TX (RX=22)

    // Timings for 100kHz @ 143.75MHz as given by CubeMX
    I2C4->TIMINGR = 0x20B0D8FF;
    I2C4->CR2 |= I2C_CR2_AUTOEND;
    I2C4->CR1 |= I2C_CR1_PE | I2C_CR1_TCIE | I2C_CR1_STOPIE;

    NVIC_SetPriority(I2C4_EV_IRQn, 16);
    NVIC_EnableIRQ(I2C4_EV_IRQn);

    SSD1306_Init(OLED_InitSendData);

    lastTick = sys_now();
    bootTimer = sys_now();
}

void OLED_Tick() {
    if (oledState.State == OLEDM_Offline) {
        return;
    }

    if (dfuTimer != 0) {
        if (sys_now() - dfuTimer > 500) {
            PWR->CR1 |= PWR_CR1_DBP;
            TAMP->BKP0R = 0xF0;
            PWR->CR1 &= ~PWR_CR1_DBP;
            NVIC_SystemReset();
            return;
        }
    }

    if (oledState.ActiveScreen == &oledScreens[0]) {
        if (sys_now() - lastTick > 250) {
            forceRefresh = 1;
        }

        if (sys_now() - bootTimer > 2500) {
            oledState.ActiveScreen = &oledScreens[1];
            forceRefresh = 1;
        }
    }

    if (oledState.OverlayTimeout != 0) {
        if (sys_now() - oledState.OverlayTimeout > 5000) {
            oledState.OverlayTimeout = 0;
            oledState.OnRender = NULL;
        }
    }

    if (sys_now() - lastInputTick > 10) {
        OLED_Buttons btn = OLEDB_None;
        char repeat = 0;

        if (GPIOB->IDR & (1 << 1)) {
            btn = OLEDB_Confirm;
        } else if (GPIOB->IDR & (1 << 2)) {
            btn = OLEDB_Down;
        } else if (GPIOB->IDR & (1 << 13)) {
            btn = OLEDB_Up;
        } else if (GPIOB->IDR & (1 << 14)) {
            btn = OLEDB_Back;
        }

        if (btn == OLEDB_None || btn != oledState.LastButton) {
            inputRepeatTimer = 500;
        } else if ((sys_now() - lastInputRepeatTick) > inputRepeatTimer && btn != OLEDB_None) {
            repeat = 1;
            inputRepeatTimer = 100;
        }

        if (btn != oledState.LastButton || repeat) {
            if (oledState.State == OLEDM_Active) {
                if (oledState.OverlayTimeout != 0) {
                    oledState.OnRender = NULL;
                    oledState.OverlayTimeout = 0;
                } else if (oledState.OnNavigate != NULL) {
                    oledState.OnNavigate(btn);
                } else if (oledState.ActiveScreen->OnNavigate != NULL) {
                    oledState.ActiveScreen->OnNavigate(btn);
                }
            }

            forceRefresh = 1;
            lastInputRepeatTick = sys_now();
        }

        lastInputTick = sys_now();
        oledState.LastButton = btn;
    }

    if (forceRefresh) {
        if (oledState.State == OLEDM_Sleeping) {
            SSD1306_Sleep(0);
            oledState.State = OLEDM_Active;
        }

        SSD1306_ClearBuffer();
        SSD1306_DrawBorder(0, 0, 128, 64, 1, 1);
        if (oledState.OnRender != NULL) {
            oledState.OnRender();
        } else {
            oledState.ActiveScreen->OnRender();
        }
        SSD1306_SendBuffer();

        lastTick = sys_now();
    }

    if (sys_now() - lastTick > 10000) {
        if (oledState.State == OLEDM_Active) {
            SSD1306_Sleep(1);
            oledState.State = OLEDM_Sleeping;
            oledState.ActiveScreen = &oledScreens[1];
        }
        lastTick = sys_now();
    }

    forceRefresh = 0;
}

void OLED_ForceRefresh() {
    forceRefresh = 1;
}

static void OLED_InitSendData(const unsigned char *buffer, unsigned short len, void (*callback)()) {
    I2C4->CR1 &= ~(I2C_CR1_RXDMAEN | I2C_CR1_TXDMAEN);
    I2C4->CR1 |= I2C_CR1_TXDMAEN;

    I2C4->CR2 &= ~(I2C_CR2_SADD_Msk);
    I2C4->CR2 |= (OLED_I2CADDR << 1);

    DMA2_Channel1->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
    DMA2_Channel1->CPAR = (uint32_t)&I2C4->TXDR;

    OLED_SendData(buffer, len);

    I2C4->CR2 |= I2C_CR2_START;

    pending_callback = callback;
}

static void OLED_SendData(const unsigned char *buffer, unsigned short len) {
    I2C4->CR2 &= ~(I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD);

    if (len <= 0xFF) {
        I2C4->CR2 |= (len << I2C_CR2_NBYTES_Pos);
        DMA2_Channel1->CNDTR = len;

        pending_buffer = NULL;
        pending_len = 0;
    } else {
        I2C4->CR2 |= (0xFF << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD;
        DMA2_Channel1->CNDTR = 0xFF;

        pending_buffer = buffer + 0xFF;
        pending_len = len - 0xFF;
    }

    DMA2_Channel1->CMAR = (uint32_t)buffer;
    DMA2_Channel1->CCR |= DMA_CCR_EN;
}

void I2C4_EV_IRQHandler() {
    if (I2C4->ISR & I2C_ISR_TCR) {
        OLED_SendData(pending_buffer, pending_len);
    }

    if (I2C4->ISR & I2C_ISR_STOPF) {
        I2C4->ICR = I2C_ICR_STOPCF;

        if (pending_callback != NULL) {
            pending_callback();
        }
    }
}

static void OLED_RenderEditFrame(char *title) {
    unsigned short width;
    unsigned short height;

    SSD1306_MeasureString(title, 15, &width, &height, &Dialog_plain_12);
    SSD1306_FillRectangle(0, 0, 128, height + 8, 1);
    SSD1306_DrawString(title, 15, 64 - width / 2, height + 2, &Dialog_plain_12, 0);
}

static void OLED_RenderBoot() {
    SSD1306_ClearBuffer();
    SSD1306_DrawBitmap(bmp_Bootlogo, sizeof(bmp_Bootlogo), 64 - 52 / 2, 32 - 52 / 2, 51, 53);
}

static void OLED_RenderDFU() {
    dfuTimer = sys_now();
    if (dfuTimer == 0)
        dfuTimer--;

    SSD1306_ClearBuffer();

    unsigned short width;
    unsigned short height;

    SSD1306_MeasureString("DFU Mode", 9, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString("DFU Mode", 9, 64 - width / 2, 32 + height / 2, &Dialog_plain_12, 1);
}

static void OLED_RenderIpOverview() {
    struct netif *netif = Config_GetNetif();
    unsigned short width;
    unsigned short height;

    char *ip = ip4addr_ntoa(&netif->ip_addr);
    SSD1306_MeasureString(ip, 15, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString(ip, 15, 64 - width / 2, height + 4, &Dialog_plain_12, 1);

    ip = ip4addr_ntoa(&netif->netmask);
    SSD1306_MeasureString(ip, 15, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString(ip, 15, 64 - width / 2, (height + 4) * 2, &Dialog_plain_12, 1);

    SSD1306_MeasureString("artnet.local", 12, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString("artnet.local", 12, 64 - width / 2, (height + 4) * 3, &Dialog_plain_12, 1);
}

static void OLED_RenderMenu() {
    signed char next = oledState.SelectedMenuItem + 1;
    signed char prev = oledState.SelectedMenuItem - 1;
    unsigned short width;
    unsigned short height;

    next = (next + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;
    prev = (prev + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;

    SSD1306_MeasureString(oledState.ActiveScreen->ListItems[prev], 15, &width, &height, &Dialog_plain_10);
    SSD1306_DrawString(oledState.ActiveScreen->ListItems[prev], 15, 64 - width / 2, 10 + height / 2, &Dialog_plain_10, 1);

    SSD1306_MeasureString(oledState.ActiveScreen->ListItems[oledState.SelectedMenuItem], 15, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString(oledState.ActiveScreen->ListItems[oledState.SelectedMenuItem], 15, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);

    SSD1306_MeasureString(oledState.ActiveScreen->ListItems[next], 15, &width, &height, &Dialog_plain_10);
    SSD1306_DrawString(oledState.ActiveScreen->ListItems[next], 15, 64 - width / 2, 50 + height / 2, &Dialog_plain_10, 1);

    SSD1306_FillRectangle(32, 24, 64, 1, 1);
    SSD1306_FillRectangle(32, 40, 64, 1, 1);
}

static void OLED_RenderEdit() {
    unsigned short width;
    unsigned short height;
    unsigned char i = oledState.SelectedMenuItem;
    OLED_EditFieldMetadata *field;

    OLED_RenderEditFrame(oledState.ActiveScreen->ListItems[i]);

    field = &oledState.EditInfo.Fields[i];
    const char *value = field->Value;

    if (field->Mode == OLEDE_List) {
        value = field->ValueList[field->ValueIndex];
    }

    SSD1306_MeasureString(value, 64, &width, &height, &Dialog_plain_12);
    if (width < 120) {
        SSD1306_DrawString(value, 64, 64 - width / 2, 50, &Dialog_plain_12, 1);
    } else {
        SSD1306_DrawString(value, 64, 4, 50, &Dialog_plain_12, 1);
    }
}

static void OLED_RenderEditSelect() {
    unsigned short width;
    unsigned short height;
    unsigned char i = oledState.SelectedMenuItem;
    OLED_EditFieldMetadata *field;

    OLED_RenderEditFrame(oledState.ActiveScreen->ListItems[i]);

    field = &oledState.EditInfo.Fields[i];

    unsigned short charWidth;
    SSD1306_MeasureString(&field->Value[oledState.EditInfo.CursorPosition], 1, &charWidth, &height, &Dialog_plain_12);
    SSD1306_MeasureString(field->Value, oledState.EditInfo.CursorPosition, &width, &height, &Dialog_plain_12);

    SSD1306_DrawStringHighlighted(field->Value, 64, oledState.EditInfo.CursorPosition, 64 - width - charWidth / 2, 50, &Dialog_plain_12, 1);
}

static void OLED_RenderValueSelect() {
    unsigned short width;
    unsigned short height;
    OLED_RenderEditFrame(oledState.ActiveScreen->ListItems[oledState.SelectedMenuItem]);

    SSD1306_FillRectangle(16, 16, 64 + 32, 32, 1);
    SSD1306_FillRectangle(17, 17, 64 + 30, 30, 0);

    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[oledState.SelectedMenuItem];

    if (field->Mode == OLEDE_List) {
        SSD1306_MeasureString(field->ValueList[oledState.EditInfo.SelectedValue], 15, &width, &height, &Dialog_plain_12);
        SSD1306_DrawString(field->ValueList[oledState.EditInfo.SelectedValue], 15, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);
    } else {
        SSD1306_MeasureString(&oledState.EditInfo.SelectedChar, 1, &width, &height, &Dialog_plain_12);
        SSD1306_DrawString(&oledState.EditInfo.SelectedChar, 1, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);
    }
}

static void OLED_RenderConfirm() {
    unsigned short width;
    unsigned short height;

    OLED_RenderEditFrame("Confirm?");

    char *value;

    if (oledState.EditInfo.SelectedValue == 0) {
        value = "No";
    } else {
        value = "Yes";
    }

    SSD1306_MeasureString(value, 64, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString(value, 64, 64 - width / 2, 50, &Dialog_plain_12, 1);
}

static void OLED_RenderInvalidValue() {
    unsigned short width;
    unsigned short height;

    char *value = "Invalid Value";

    SSD1306_MeasureString(value, 64, &width, &height, &Dialog_plain_12);
    SSD1306_DrawString(value, 64, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);
}

static void OLED_NavigateMenu(OLED_Buttons btn) {
    switch (btn) {
    case OLEDB_Up:
        if (oledState.ActiveScreen->ListSize > 0) {
            oledState.SelectedMenuItem--;
            oledState.SelectedMenuItem = (oledState.SelectedMenuItem + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;
        }
        break;
    case OLEDB_Down:
        if (oledState.ActiveScreen->ListSize > 0) {
            oledState.SelectedMenuItem++;
            oledState.SelectedMenuItem = (oledState.SelectedMenuItem + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;
        }
        break;
    case OLEDB_Confirm: {
        // find next screen
        for (int i = 0; i < oledTransitions; i++) {
            if (oledTransitionMap[i].Source == oledState.ActiveScreen) {
                if (oledTransitionMap[i].MenuItem == -1 || oledTransitionMap[i].MenuItem == oledState.SelectedMenuItem) {
                    oledState.ActiveScreen = oledTransitionMap[i].Target;
                    oledState.ActiveScreen->ScreenParameter = oledTransitionMap[i].Parameter;
                    oledState.SelectedMenuItem = 0;
                    oledState.EditInfo.Changed = 0;
                    oledState.EditInfo.SelectedValue = 0;
                    oledState.OnNavigate = NULL;
                    oledState.OnRender = NULL;

                    if (oledState.ActiveScreen->OnInit != NULL) {
                        oledState.ActiveScreen->OnInit();
                    }

                    break;
                }
            }
        }
    } break;
    case OLEDB_Back:
        // find previous screen
        for (int i = 0; i < oledTransitions; i++) {
            if (oledTransitionMap[i].Target == oledState.ActiveScreen) {
                if (oledTransitionMap[i].Parameter == oledState.ActiveScreen->ScreenParameter) {
                    oledState.ActiveScreen = oledTransitionMap[i].Source;
                    oledState.SelectedMenuItem = oledTransitionMap[i].MenuItem;
                    oledState.OnRender = NULL;
                    oledState.OnNavigate = NULL;

                    if (oledState.SelectedMenuItem < 0) {
                        oledState.SelectedMenuItem = 0;
                    }

                    if (oledState.ActiveScreen->OnInit != NULL) {
                        oledState.ActiveScreen->OnInit();
                    }

                    break;
                }
            }
        }
        break;
    default:
        // No button, do nothing
        break;
    }
}

static void OLED_NavigateValueSelect(OLED_Buttons btn) {
    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[oledState.SelectedMenuItem];

    switch (btn) {
    case OLEDB_Confirm:
        if (field->Mode == OLEDE_List) {
            field->ValueIndex = oledState.EditInfo.SelectedValue;
        } else {
            field->Value[oledState.EditInfo.CursorPosition] = oledState.EditInfo.SelectedChar;
        }

        oledState.EditInfo.Changed = 1;

        // No break, return to previous screen as if back was pressed
    case OLEDB_Back:
        oledState.OnRender = OLED_RenderEditSelect;
        oledState.OnNavigate = OLED_NavigateEditSelect;

        if (field->Mode == OLEDE_List) {
            oledState.OnRender = NULL;
            oledState.OnNavigate = NULL;
        }
        break;
    case OLEDB_Up:
        oledState.EditInfo.SelectedValue++;
        break;
    case OLEDB_Down:
        oledState.EditInfo.SelectedValue--;
        break;
    default:
        break;
    }

    char min = 0x00;
    char max = 0xFF;

    switch (field->Mode) {
    case OLEDE_List:
        min = 0;
        max = field->ValueListSize - 1;
        break;
    case OLEDE_Int:
    case OLEDE_Ip:
        min = '0';
        max = '9';
        break;
    case OLEDE_Text:
        min = ' ';
        max = 'z';
        break;
    }

    if (oledState.EditInfo.SelectedValue < min)
        oledState.EditInfo.SelectedValue = max;

    if (oledState.EditInfo.SelectedValue > max)
        oledState.EditInfo.SelectedValue = min;
}

static void OLED_NavigateEditSelect(OLED_Buttons btn) {
    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[oledState.SelectedMenuItem];

    switch (btn) {
    case OLEDB_Confirm:
        if (field->Mode == OLEDE_Ip && (oledState.EditInfo.CursorPosition % 4) == 3)
            break;

        oledState.OnRender = OLED_RenderValueSelect;
        oledState.OnNavigate = OLED_NavigateValueSelect;
        oledState.EditInfo.SelectedValue = field->Value[oledState.EditInfo.CursorPosition];

        break;
    case OLEDB_Back:
        if (OLED_ValidateFieldValue(oledState.SelectedMenuItem)) {
            oledState.OverlayTimeout = sys_now();
            if (oledState.OverlayTimeout == 0)
                oledState.OverlayTimeout--;
            oledState.OnRender = OLED_RenderInvalidValue;
            memcpy(field->Value, field->ValueCopy, 64);
        }

        oledState.OnRender = NULL;
        oledState.OnNavigate = NULL;

        if (field->Mode == OLEDE_Text) {
            for (int i = field->FieldLength - 1; i >= 0; i--) {
                if (field->Value[i] == ' ') {
                    field->Value[i] = 0;
                } else {
                    break;
                }
            }
        }

        break;
    case OLEDB_Up:
        oledState.EditInfo.CursorPosition--;
        break;
    case OLEDB_Down:
        oledState.EditInfo.CursorPosition++;
        break;
    default:
        break;
    }

    oledState.EditInfo.CursorPosition = (oledState.EditInfo.CursorPosition + field->FieldLength) % field->FieldLength;
}

static void OLED_NavigateEdit(OLED_Buttons btn) {
    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[oledState.SelectedMenuItem];

    if (btn == OLEDB_Confirm) {
        if (field->Mode != OLEDE_List) {
            oledState.OnRender = OLED_RenderEditSelect;
            oledState.OnNavigate = OLED_NavigateEditSelect;
            oledState.EditInfo.CursorPosition = 0;

            if (field->Mode == OLEDE_Text) {
                for (int i = 0; i < field->FieldLength; i++) {
                    if (field->Value[i] == 0)
                        field->Value[i] = ' ';
                }
            }
        } else {
            oledState.OnRender = OLED_RenderValueSelect;
            oledState.OnNavigate = OLED_NavigateValueSelect;
            oledState.EditInfo.SelectedValue = field->ValueIndex;
        }

        memcpy(field->ValueCopy, field->Value, 64);
    } else {
        if (btn == OLEDB_Back && oledState.EditInfo.Changed) {
            oledState.OnRender = OLED_RenderConfirm;
            oledState.OnNavigate = OLED_NavigateConfirm;
            oledState.EditInfo.SelectedValue = 0;
        } else {
            OLED_NavigateMenu(btn);
        }
    }
}

static void OLED_NavigateConfirm(OLED_Buttons btn) {
    switch (btn) {
    case OLEDB_Confirm:
        if (oledState.EditInfo.SelectedValue == 1) {
            if (oledState.ActiveScreen->OnConfirm != NULL) {
                oledState.ActiveScreen->OnConfirm();
            }
        }

        OLED_NavigateMenu(OLEDB_Back);
        break;
    case OLEDB_Back:
        if (oledState.OnRender == NULL) {
            OLED_NavigateMenu(OLEDB_Back);
        } else {
            oledState.OnNavigate = NULL;
            oledState.OnRender = NULL;
        }
        break;
    case OLEDB_Up:
    case OLEDB_Down:
        oledState.EditInfo.SelectedValue = (oledState.EditInfo.SelectedValue + 1) % 2;
        break;
    default:
        break;
    }
}

static char OLED_ValidateFieldValue(unsigned char i) {
    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[i];

    switch (field->Mode) {
    case OLEDE_Ip: {
        unsigned short part;
        for (int i = 0; i < 4; i++) {
            part = atoi(&field->Value[i * 4]);
            if (part > 255) {
                return 1;
            }
        }

        return 0;
    }
    case OLEDE_Int: {
        short value = atoi(field->Value);

        if (value < field->Minimum || value > field->Maximum) {
            return 1;
        } else {
            return 0;
        }
    }
    default:
        return 0;
    }
}

/*
--------------------------------------------------------------
-- Initialize Fields for Editing
--------------------------------------------------------------
*/
static void OLED_SetFieldIp(unsigned char i, ip_addr_t *addr) {
    char ip[4] = {
        ip4_addr1(addr),
        ip4_addr2(addr),
        ip4_addr3(addr),
        ip4_addr4(addr)};

    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Ip;
    oledState.EditInfo.Fields[i].FieldLength = 15;

    snprintf(oledState.EditInfo.Fields[i].Value, 16, "%03u.%03u.%03u.%03u", ip[0], ip[1], ip[2], ip[3]);
}

static void OLED_GetFieldIp(unsigned char i, ip_addr_t *addr) {
    // Need to parse the IP block by block, because a leading 0 indicates base 8 in ipaddr_aton
    OLED_EditFieldMetadata *field = &oledState.EditInfo.Fields[i];
    IP4_ADDR(addr, atoi(field->Value), atoi(&field->Value[4]), atoi(&field->Value[8]), atoi(&field->Value[12]));
}

static void OLED_SetFieldList(unsigned char i, unsigned char value, const char **list, unsigned char listSize) {
    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_List;
    oledState.EditInfo.Fields[i].ValueIndex = value;
    oledState.EditInfo.Fields[i].ValueList = list;
    oledState.EditInfo.Fields[i].ValueListSize = listSize;
}

static char OLED_GetFieldList(unsigned char i) {
    return oledState.EditInfo.Fields[i].ValueIndex;
}

static void OLED_SetFieldInt(unsigned char i, short value, short min, short max, signed char digits) {
    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Int;
    oledState.EditInfo.Fields[i].Minimum = min;
    oledState.EditInfo.Fields[i].Maximum = max;
    oledState.EditInfo.Fields[i].FieldLength = digits;

    if (digits < 0) {
        snprintf(oledState.EditInfo.Fields[i].Value, 15, "%d", value);
    } else {
        char buffer[10];
        snprintf(buffer, 10, "%%0%uu", digits);
        snprintf(oledState.EditInfo.Fields[i].Value, 15, buffer, value);
    }
}

static short OLED_GetFieldInt(unsigned char i) {
    return atoi(oledState.EditInfo.Fields[i].Value);
}

static void OLED_SetFieldStr(unsigned char i, char *str, short maxLen) {
    if (maxLen < 0)
        maxLen = 64;

    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Text;
    oledState.EditInfo.Fields[i].FieldLength = maxLen;
    strncpy(oledState.EditInfo.Fields[i].Value, str, maxLen);
}

static void OLED_GetFieldStr(unsigned char i, char *str, unsigned short maxLen) {
    if (maxLen > 64)
        return;

    strncpy(str, oledState.EditInfo.Fields[i].Value, maxLen);
}

static void OLED_InitPort() {
    ARTNET_CONFIG *cfg = &Config_GetActive()->ArtNet[oledState.ActiveScreen->ScreenParameter];
    short universe = cfg->Universe & 0x0F;
    universe |= (cfg->Subnet & 0x0F) << 4;
    universe |= (cfg->Network & 0x7F) << 8;

    OLED_SetFieldStr(0, cfg->ShortName, 18);
    OLED_SetFieldStr(1, cfg->LongName, 64);
    OLED_SetFieldList(2, cfg->PortDirection == ARTNET_OUTPUT ? 0 : 1, &opt_portDir, 2);
    OLED_SetFieldInt(3, universe, 0, 32767, 5);
    OLED_SetFieldInt(4, cfg->AcnPriority, 0, 254, 3);
    OLED_SetFieldList(5, (cfg->PortFlags & PORT_FLAG_INDISABLED) != 0 ? 0 : 1, &opt_disabled, 2); // flipped
    OLED_SetFieldList(6, (cfg->PortFlags & PORT_FLAG_RDM) != 0 ? 1 : 0, &opt_disabled, 2);
    OLED_SetFieldList(7, (cfg->PortFlags & PORT_FLAG_SINGLE) != 0 ? 1 : 0, &opt_portMode, 2);
    OLED_SetFieldList(8, cfg->FailoverMode, &opt_failover, 4);
    OLED_SetFieldList(9, 0, &opt_yesno, 2);
}

static void OLED_InitIp() {
    CONFIG *cfg = Config_GetActive();

    OLED_SetFieldList(0, cfg->Mode, &opt_ipMode, 3);
    OLED_SetFieldIp(1, &cfg->StaticIp);
    OLED_SetFieldIp(2, &cfg->StaticSubnet);
    OLED_SetFieldIp(3, &cfg->StaticGateway);
    OLED_SetFieldList(4, cfg->DhcpServerEnable, &opt_disabled, 2);
    OLED_SetFieldIp(5, &cfg->DhcpServerSelf);
    OLED_SetFieldIp(6, &cfg->DhcpServerClient);
    OLED_SetFieldIp(7, &cfg->DhcpServerSubnet);
}

static void OLED_InitFirmware() {
    OLED_SetFieldStr(0, FIRMWARE_VER, -1);
    OLED_SetFieldInt(1, FIRMWARE_INT, -1, -1, -1);

    oledState.EditInfo.Fields[0].Disabled = 1;
    oledState.EditInfo.Fields[1].Disabled = 1;
}

static void OLED_ConfirmPort() {
    ARTNET_CONFIG *cfg = &Config_GetActive()->ArtNet[oledState.ActiveScreen->ScreenParameter];

    OLED_GetFieldStr(0, cfg->ShortName, 18);
    OLED_GetFieldStr(1, cfg->LongName, 64);
    if (OLED_GetFieldList(2)) {
        cfg->PortDirection = ARTNET_INPUT;
    } else {
        cfg->PortDirection = ARTNET_OUTPUT;
    }

    short universe = OLED_GetFieldInt(3);
    cfg->Universe = universe & 0x0F;
    cfg->Subnet = (universe >> 4) & 0x0F;
    cfg->Network = (universe >> 8) & 0x7F;

    cfg->AcnPriority = OLED_GetFieldInt(4);
    if (OLED_GetFieldList(5)) { // flipped
        cfg->PortFlags &= ~PORT_FLAG_INDISABLED;
    } else {
        cfg->PortFlags |= PORT_FLAG_INDISABLED;
    }
    if (OLED_GetFieldList(6)) {
        cfg->PortFlags |= PORT_FLAG_RDM;
    } else {
        cfg->PortFlags &= ~PORT_FLAG_RDM;
    }
    if (OLED_GetFieldList(7)) {
        cfg->PortFlags |= PORT_FLAG_SINGLE;
    } else {
        cfg->PortFlags &= ~PORT_FLAG_SINGLE;
    }
    cfg->FailoverMode = OLED_GetFieldList(8);

    if (OLED_GetFieldList(9)) {
        Config_StoreFailsafeScene(DMX_GetBuffer(oledState.ActiveScreen->ScreenParameter), oledState.ActiveScreen->ScreenParameter);
    }

    Config_ApplyArtNet();
    Config_Store();
}

static void OLED_ConfirmIp() {
    CONFIG *cfg = Config_GetActive();

    cfg->Mode = OLED_GetFieldList(0);
    OLED_GetFieldIp(1, &cfg->StaticIp);
    OLED_GetFieldIp(2, &cfg->StaticSubnet);
    OLED_GetFieldIp(3, &cfg->StaticGateway);
    cfg->DhcpServerEnable = OLED_GetFieldList(4);
    OLED_GetFieldIp(5, &cfg->DhcpServerSelf);
    OLED_GetFieldIp(6, &cfg->DhcpServerClient);
    OLED_GetFieldIp(7, &cfg->DhcpServerSubnet);

    Config_ApplyNetwork();
    Config_Store();
}

static void OLED_ConfirmReset() {
    Config_Reset();
}
