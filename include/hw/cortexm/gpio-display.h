/*
 * GPIO connected 7-segment LED display
 *
 * Copyright (c) 2017 Doulos.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GPIO_DISPLAY_H_
#define GPIO_DISPLAY_H_

#include "qemu/osdep.h"

#include "hw/qdev.h"
#include "qemu/typedefs.h"
#include "hw/sysbus.h"

#include <hw/cortexm/graphic.h>

// ----------------------------------------------------------------------------
//#define CONFIG_SDLABI_2_0
//#define CONFIG_SDL

#define IRQ_GPIO_DISPLAY_IN     "display-in"

// ----------------------------------------------------------------------------

#define DEFINE_PROP_GPIO_DISPLAY_PTR(_n, _s, _f) \
    DEFINE_PROP(_n, _s, _f, qdev_prop_ptr, GPIODISPLAYInfo*)

// ----------------------------------------------------------------------------

typedef struct {

    const char *name; /* NULL for table end. */

//    bool active_low;
    const char *colour_name;
/*
    const char *on_message;
    const char *off_message;
*/

    /* Centre of the LED Segment. */
    uint32_t x;
    uint32_t y;
    /* Size of the LED segment rectangle */
    uint32_t w;
    uint32_t h;

    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } colour;

    const char *gpio_path;
    const char *irq_name;
    int gpio_bit;

} GPIODISPLAYInfo;

// ----------------------------------------------------------------------------

#define TYPE_GPIO_DISPLAY "gpio-display"

// Parent definitions.
// TODO: Change this to TYPE_DEVICE
#define TYPE_GPIO_DISPLAY_PARENT TYPE_SYS_BUS_DEVICE
typedef SysBusDeviceClass GPIODISPLAYParentClass;
typedef SysBusDevice GPIODISPLAYParentState;

// ----------------------------------------------------------------------------

// Class definitions.
#define GPIO_DISPLAY_GET_CLASS(obj) \
    OBJECT_GET_CLASS(GPIODISPLAYClass, (obj), TYPE_GPIO_DISPLAY)
#define GPIO_DISPLAY_CLASS(klass) \
    OBJECT_CLASS_CHECK(GPIODISPLAYClass, (klass), TYPE_GPIO_DISPLAY)

typedef struct {
    // private:
    GPIODISPLAYParentClass parent_class;
    // public:

} GPIODISPLAYClass;

// ----------------------------------------------------------------------------

// Instance definitions.
#define GPIO_DISPLAY_STATE(obj) \
    OBJECT_CHECK(GPIODISPLAYState, (obj), TYPE_GPIO_DISPLAY)

/*
 * The LED has a single incoming interrupt; connect the source,
 * usually a GPIO outgoing interrupt, to it.
 *
 * Properties:
 * - active-low (bool)
 * - on-message (string)
 * - off-message (string)
 * - x,y,w,h (int)
 * - colour.red, colour.green, colour.blue (int)
 */
typedef struct {
    // private:
    GPIODISPLAYParentState parent_obj;
    // public:

//    bool active_low;
//    const char *on_message;
//    const char *off_message;

#if defined(CONFIG_SDL)
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } colour;
    DISPLAYGraphicContext display_graphic_context;
    BoardGraphicContext *board_graphic_context;
#endif /* defined(CONFIG_SDL) */

} GPIODISPLAYState;

Object **gpio_display_create_from_info(Object *parent, GPIODISPLAYInfo *info_array,
        BoardGraphicContext *graphic_context);

// ----------------------------------------------------------------------------

#endif /* GPIO_DISPLAY_H_ */
