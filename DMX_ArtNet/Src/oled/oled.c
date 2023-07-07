#include "oled/oled.h"
#include "config.h"
#include "dmx_usart.h"
#include "lwip/netif.h"
#include "oled/fonts.h"

static void OLED_InitSendData(const unsigned char *buffer, unsigned short len, void (*callback)());
static void OLED_SendData(const unsigned char *buffer, unsigned short len);

static void OLED_RenderBoot();
static void OLED_RenderDFU();
static void OLED_RenderIpOverview();
static void OLED_RenderMenu();
static void OLED_RenderEdit();

static void OLED_NavigateEdit(OLED_Buttons btn);
static void OLED_NavigateMenu(OLED_Buttons btn);

static void OLED_InitPort();
static void OLED_InitIp();
static void OLED_InitFirmware();

static const unsigned char *pending_buffer;
static unsigned short pending_len;
static void (*pending_callback)();

static unsigned int lastTick;
static unsigned int bootTimer;
static unsigned int lastInputTick;

static char *opt_disabled[] = {"Disabled", "Enabled"};
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
    // Ports - 9
    "Name",
    "Description",
    "Direction",
    "Universe",
    "ACN Prio",
    "In Disable",
    "RDM",
    "Port Mode",
    "Failover",
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
    // Confirm Dialog - 1
    "Confirm",
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
        .ListItems = &listItems[8],
        .ListSize = 9,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = OLED_NavigateEdit,
        .OnInit = OLED_InitIp,
        .ListItems = &listItems[8 + 9],
        .ListSize = 8,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = OLED_NavigateMenu,
        .OnInit = OLED_InitFirmware,
        .ListItems = &listItems[8 + 9 + 8],
        .ListSize = 2,
    },
    {
        .OnRender = OLED_RenderEdit,
        .OnNavigate = NULL,
        .OnInit = NULL,
        .ListItems = &listItems[8 + 9 + 8 + 2],
        .ListItems = 1,
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

static OLED_State oledState = {.State = OLEDM_Offline, .ActiveScreen = &oledScreens[0]};

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
    char forceRefresh = 0;

    if (oledState.State == OLEDM_Offline) {
        return;
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

    if (sys_now() - lastInputTick > 10) {
        OLED_Buttons btn = OLEDB_None;
        if (GPIOB->IDR & (1 << 1)) {
            btn = OLEDB_Confirm;
        } else if (GPIOB->IDR & (1 << 2)) {
            btn = OLEDB_Down;
        } else if (GPIOB->IDR & (1 << 13)) {
            btn = OLEDB_Up;
        } else if (GPIOB->IDR & (1 << 14)) {
            btn = OLEDB_Back;
        }

        if (btn != oledState.LastButton) {
            if (oledState.ActiveScreen->OnNavigate != NULL && oledState.State == OLEDM_Active) {
                oledState.ActiveScreen->OnNavigate(btn);
            }

            forceRefresh = 1;
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
        oledState.ActiveScreen->OnRender();
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
}

static void OLED_InitSendData(const unsigned char *buffer, unsigned short len, void (*callback)()) {
    I2C4->CR1 &= ~(I2C_CR1_RXDMAEN | I2C_CR1_TXDMAEN);
    I2C4->CR1 |= I2C_CR1_TXDMAEN;

    I2C4->CR2 &= ~(I2C_CR2_SADD_Msk);
    I2C4->CR2 |= (OLED_I2CADDR << 1);

    DMA2_Channel1->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
    DMA2_Channel1->CPAR = &I2C4->TXDR;

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

    DMA2_Channel1->CMAR = buffer;
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

// OLED Screens
static void OLED_RefreshTick() {
    // switch (oledState.RenderMode) {
    // case OLEDRM_Static:
    //     switch (oledState.Screen) {
    //     case OLEDS_PreBoot:
    //     case OLEDS_Boot:
    //         SSD1306_DrawBitmap(bmp_Bootlogo, sizeof(bmp_Bootlogo), 64 - 52 / 2, 32 - 52 / 2, 51, 53);
    //         break;
    //     case OLEDS_Overview: {
    //         SSD1306_DrawBorder(0, 0, 128, 64, 1, 1);

    //         char *ip = ip4addr_ntoa(&netif->ip_addr);
    //         SSD1306_MeasureString(ip, 15, &width, &height, &Dialog_plain_12);
    //         SSD1306_DrawString(ip, 15, 64 - width / 2, height + 4, &Dialog_plain_12, 1);

    //         ip = ip4addr_ntoa(&netif->netmask);
    //         SSD1306_MeasureString(ip, 15, &width, &height, &Dialog_plain_12);
    //         SSD1306_DrawString(ip, 15, 64 - width / 2, (height + 4) * 2, &Dialog_plain_12, 1);

    //         SSD1306_MeasureString("artnet.local", 12, &width, &height, &Dialog_plain_12);
    //         SSD1306_DrawString("artnet.local", 12, 64 - width / 2, (height + 4) * 3, &Dialog_plain_12, 1);
    //         break;
    //     }
    //     default:
    //         break;
    //     }
    //     break;
    // case OLEDRM_Menu: {
    //     SSD1306_DrawBorder(0, 0, 128, 64, 1, 1);

    //     char next = oledState.Navigation.SelectedItem + 1;
    //     char prev = oledState.Navigation.SelectedItem - 1;

    //     next = (next + oledState.Navigation.ItemCount) % oledState.Navigation.ItemCount;
    //     prev = (prev + oledState.Navigation.ItemCount) % oledState.Navigation.ItemCount;

    //     SSD1306_MeasureString(oled_menu_detail[prev], 15, &width, &height, &Dialog_plain_10);
    //     SSD1306_DrawString(oled_menu_detail[prev], 15, 64 - width / 2, 10 + height / 2, &Dialog_plain_10, 1);

    //     SSD1306_MeasureString(oled_menu_detail[oledState.Navigation.SelectedItem], 15, &width, &height, &Dialog_plain_12);
    //     SSD1306_DrawString(oled_menu_detail[oledState.Navigation.SelectedItem], 15, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);

    //     SSD1306_MeasureString(oled_menu_detail[next], 15, &width, &height, &Dialog_plain_10);
    //     SSD1306_DrawString(oled_menu_detail[next], 15, 64 - width / 2, 50 + height / 2, &Dialog_plain_10, 1);

    //     SSD1306_FillRectangle(32, 24, 64, 1, 1);
    //     SSD1306_FillRectangle(32, 40, 64, 1, 1);
    //     break;
    // }
    // case OLEDRM_Edit: {
    //     SSD1306_DrawBorder(0, 0, 128, 64, 1, 1);
    //     SSD1306_MeasureString(oledState.EditTitle, 15, &width, &height, &Dialog_plain_12);
    //     SSD1306_FillRectangle(0, 0, 128, height + 8, 1);
    //     SSD1306_DrawString(oledState.EditTitle, 15, 64 - width / 2, height + 2, &Dialog_plain_12, 0);

    //     if (!oledState.EditEnabled) {
    //         if (oledState.EditMode == OLEDE_List) {
    //             char *txt = oledState.ListEntries[oledState.SelectedItem];
    //             SSD1306_MeasureString(txt, 15, &width, &height, &Dialog_plain_12);
    //             SSD1306_DrawString(txt, 15, 64 - width / 2, 50, &Dialog_plain_12, 1);
    //         } else {
    //             SSD1306_MeasureString(oledState.EditValue, 64, &width, &height, &Dialog_plain_12);
    //             if (width < 120) {
    //                 SSD1306_DrawString(oledState.EditValue, 64, 64 - width / 2, 50, &Dialog_plain_12, 1);
    //             } else {
    //                 SSD1306_DrawString(oledState.EditValue, 64, 4, 50, &Dialog_plain_12, 1);
    //             }
    //         }
    //     } else {
    //         if (oledState.EditMode != OLEDE_List) {
    //             unsigned short charWidth;
    //             SSD1306_MeasureString(&oledState.EditValue[oledState.CursorPos], 1, &charWidth, &height, &Dialog_plain_12);
    //             SSD1306_MeasureString(oledState.EditValue, oledState.CursorPos, &width, &height, &Dialog_plain_12);

    //             SSD1306_DrawStringHighlighted(oledState.EditValue, 64, oledState.CursorPos, 64 - width - charWidth / 2, 50, &Dialog_plain_12, 1);
    //         }

    //         if (oledState.EditEnabled == 2) {
    //             SSD1306_FillRectangle(16, 16, 64 + 32, 32, 1);
    //             SSD1306_FillRectangle(17, 17, 64 + 30, 30, 0);

    //             char *txt = oledState.ListEntries[oledState.SelectedItem];
    //             SSD1306_MeasureString(txt, 15, &width, &height, &Dialog_plain_12);
    //             SSD1306_DrawString(txt, 15, 64 - width / 2, 30 + height / 2, &Dialog_plain_12, 1);
    //         }
    //     }
    // }
    // }
}

static void OLED_RenderBoot() {
    SSD1306_ClearBuffer();
    SSD1306_DrawBitmap(bmp_Bootlogo, sizeof(bmp_Bootlogo), 64 - 52 / 2, 32 - 52 / 2, 51, 53);
}

static void OLED_RenderDFU() {
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
    char next = oledState.SelectedMenuItem + 1;
    char prev = oledState.SelectedMenuItem - 1;
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

    SSD1306_MeasureString(oledState.ActiveScreen->ListItems[i], 15, &width, &height, &Dialog_plain_12);
    SSD1306_FillRectangle(0, 0, 128, height + 8, 1);
    SSD1306_DrawString(oledState.ActiveScreen->ListItems[i], 15, 64 - width / 2, height + 2, &Dialog_plain_12, 0);

    field = &oledState.EditInfo.Fields[i];
    char *value = field->Value;

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

static void OLED_NavigateMenu(OLED_Buttons btn) {
    switch (btn) {
    case OLEDB_Up:
        oledState.SelectedMenuItem--;
        oledState.SelectedMenuItem = (oledState.SelectedMenuItem + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;
        break;
    case OLEDB_Down:
        oledState.SelectedMenuItem++;
        oledState.SelectedMenuItem = (oledState.SelectedMenuItem + oledState.ActiveScreen->ListSize) % oledState.ActiveScreen->ListSize;
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
    }
}

static void OLED_NavigateEdit(OLED_Buttons btn) {
    OLED_NavigateMenu(btn);
}

static void OLED_SetFieldIp(unsigned char i, ip_addr_t *addr) {
    char ip[4] = {
        ip4_addr1(addr),
        ip4_addr2(addr),
        ip4_addr3(addr),
        ip4_addr4(addr)};

    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Ip;

    snprintf(oledState.EditInfo.Fields[i].Value, 16, "%03u.%03u.%03u.%03u", ip[0], ip[1], ip[2], ip[3]);
}

static void OLED_SetFieldList(unsigned char i, unsigned char value, char **list, unsigned char listSize) {
    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_List;
    oledState.EditInfo.Fields[i].ValueIndex = value;
    oledState.EditInfo.Fields[i].ValueList = list;
    oledState.EditInfo.Fields[i].ValueListSize = listSize;
}

static void OLED_SetFieldInt(unsigned char i, short value, short min, short max, char digits) {
    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Int;
    oledState.EditInfo.Fields[i].Minimum = min;
    oledState.EditInfo.Fields[i].Maximum = max;

    if (digits < 0) {
        snprintf(oledState.EditInfo.Fields[i].Value, 15, "%d", value);
    } else {
        char buffer[10];
        snprintf(buffer, 10, "%%0%uu", digits);
        snprintf(oledState.EditInfo.Fields[i].Value, 15, buffer, value);
    }
}

static void OLED_SetFieldStr(unsigned char i, char *str, short maxLen) {
    if (maxLen < 0)
        maxLen = 64;

    oledState.EditInfo.Fields[i].Disabled = 0;
    oledState.EditInfo.Fields[i].Mode = OLEDE_Text;
    oledState.EditInfo.Fields[i].Maximum = maxLen;
    strncpy(oledState.EditInfo.Fields[i].Value, str, maxLen);
}

static void OLED_InitPort() {
    ARTNET_CONFIG *cfg = &Config_GetActive()->ArtNet[oledState.ActiveScreen->ScreenParameter];

    OLED_SetFieldStr(0, cfg->ShortName, 18);
    OLED_SetFieldStr(1, cfg->LongName, 64);
    OLED_SetFieldList(2, cfg->PortDirection == USART_OUTPUT ? 0 : 1, &opt_portDir, 2);
    OLED_SetFieldInt(3, cfg->Universe, 0, 32000, 5);
    OLED_SetFieldInt(4, cfg->AcnPriority, 0, 254, 3);
    OLED_SetFieldList(5, (cfg->PortFlags & PORT_FLAG_INDISABLED) != 0 ? 0 : 1, &opt_disabled, 2);
    OLED_SetFieldList(6, (cfg->PortFlags & PORT_FLAG_RDM) != 0 ? 1 : 0, &opt_disabled, 2);
    OLED_SetFieldList(7, (cfg->PortFlags & PORT_FLAG_SINGLE) != 0 ? 1 : 0, &opt_portMode, 2);
    OLED_SetFieldList(8, cfg->FailoverMode, &opt_failover, 4);
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