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

#include <hw/cortexm/gpio-display.h>
#include <hw/cortexm/helper.h>

#if defined(CONFIG_VERBOSE)
#include "verbosity.h"
#endif

/*
 * This class implements a 7-segment LED display connected to a GPIO device.
 */

// ----- Public ---------------------------------------------------------------
// Create a number of LED segments, using details from an array of Info structures.
Object **gpio_display_create_from_info(Object *parent, GPIODISPLAYInfo *info_array,
        BoardGraphicContext *graphic_context)
{
    GPIODISPLAYInfo *info;

    int count = 0;
    for (info = info_array; info->name; info++) {
        count++;
    }

    Object **arr;
    arr = g_malloc0_n(count + 1, sizeof(Object*));

    Object **p = arr;

    for (info = info_array; info->name; info++) {
        // Create the board DISPLAY
        Object *display = cm_object_new(parent, info->name, TYPE_GPIO_DISPLAY);

        //cm_object_property_set_bool(display, info->active_low, "active-low");

//        const char *msg = NULL;
//        size_t len;
//        if (info->on_message) {
//            msg = info->on_message;
//        } else {
//            len = strlen(info->name) + 16;
//            msg = g_malloc(len);
//            snprintf((char * )msg, len, "[%s on]\n", info->name);
//        }
//        cm_object_property_set_str(display, msg, "on-message");
//
//        msg = NULL;
//        if (info->off_message) {
//            msg = info->off_message;
//        } else {
//            len = strlen(info->name) + 16;
//            msg = g_malloc(len);
//            snprintf((char * )msg, len, "[%s off]\n", info->name);
//        }
//        cm_object_property_set_str(display, msg, "off-message");


#if defined(CONFIG_SDL)

        if (info->w && info->h) {
            // Compute corner coordinate from centre coordinate.
            cm_object_property_set_int(display, info->x - (info->w / 2), "x");
            cm_object_property_set_int(display, info->y - (info->h / 2), "y");
            cm_object_property_set_int(display, info->w, "w");
            cm_object_property_set_int(display, info->h, "h");
        }

        if (info->colour.red == 0 && info->colour.green == 0
                && info->colour.blue == 0) {

            if (strcasecmp(info->colour_name, "red") == 0) {
                cm_object_property_set_int(display, 255, "colour.red");
            } else if (strcasecmp(info->colour_name, "green") == 0) {
                cm_object_property_set_int(display, 255, "colour.green");
            } else if (strcasecmp(info->colour_name, "blue") == 0) {
                cm_object_property_set_int(display, 255, "colour.blue");
                cm_object_property_set_int(display, 128, "colour.green");
            } else if (strcasecmp(info->colour_name, "yellow") == 0) {
                cm_object_property_set_int(display, 255, "colour.red");
                cm_object_property_set_int(display, 255, "colour.green");
            } else if (strcasecmp(info->colour_name, "orange") == 0) {
                cm_object_property_set_int(display, 255, "colour.red");
                cm_object_property_set_int(display, 153, "colour.green");
                cm_object_property_set_int(display, 51, "colour.blue");
            } else {
                // White DISPLAY
                cm_object_property_set_int(display, 255, "colour.red");
                cm_object_property_set_int(display, 255, "colour.green");
                cm_object_property_set_int(display, 255, "colour.blue");
            }

        }
        // Remember the graphic context in each DISPLAY
        // Remember the board graphic context in each DISPLAY.
        GPIO_DISPLAY_STATE(display)->board_graphic_context = graphic_context;

#endif /* defined(CONFIG_SDL) */

        cm_object_realize(display);

        if (info->gpio_path) {
            if (info->irq_name == NULL) {
                fprintf(stderr,
                        "Missing mandatory irq_name in '%s' definition\n",
                        info->name);
                exit(1);
            }
            // Connect the outgoing interrupt of the GPIO bit to the (only)
            // incoming interrupt of this DISPLAY.
            cm_irq_connect(cm_device_by_name(info->gpio_path), info->irq_name,
                    info->gpio_bit, DEVICE(display), IRQ_GPIO_DISPLAY_IN, 0);
        }

#if defined(CONFIG_VERBOSE)
        if (verbosity_level >= VERBOSITY_DETAILED) {
            printf("'%s'", object_get_canonical_path(display));
            if (info->w && info->h) {
                printf(" %d*%d @(%d,%d)", info->w, info->h, info->x, info->y);
            }
//            printf(" active %s", info->active_low ? "low" : "high");
//            if (info->gpio_path) {
//                printf(" '%s',%d", info->gpio_path, info->gpio_bit);
//            }
            printf("\n");
        }
#endif /* defined(CONFIG_VERBOSE) */

        *p++ = display;
    }

    return arr;
}

// ----- Private --------------------------------------------------------------

#define DISPLAY_ON true
#define DISPLAY_OFF false

static void gpio_display_turn(GPIODISPLAYState *state, bool is_on)
{
//    printf("%s", is_on ? state->on_message : state->off_message);

#if defined(CONFIG_SDL)

    if (state->board_graphic_context != NULL) {
        cortexm_graphic_enqueue_event(GRAPHIC_EVENT_DISPLAY_TURN, state,
                (void*) is_on);
    }

#endif /* defined(CONFIG_SDL) */
}

// Callback used to notify the DISPLAY status change.
static void gpio_display_irq_handler(void *opaque, int n, int level)
{
    GPIODISPLAYState *state = GPIO_DISPLAY_STATE(opaque);

    // There should be only one IRQ for the DISPLAY
    assert(n == 0);

    // Assume that the IRQ is only triggered if the DISPLAY has changed state.
    // If this is not correct, we may get multiple DISPLAY Off's or On's in a row.
    switch (level) {
    case 0:

        if (state->active_low) {
            gpio_display_turn(state, DISPLAY_ON);
        } else {
            gpio_display_turn(state, DISPLAY_OFF);
        }
        break;

    case 1:
        if (state->active_low) {
            gpio_display_turn(state, DISPLAY_OFF);
        } else {
            gpio_display_turn(state, DISPLAY_ON);
        }
        break;
    }
}

static void gpio_display_instance_init_callback(Object *obj)
{
    qemu_log_function_name();

    GPIODISPLAYState *state = GPIO_DISPLAY_STATE(obj);

//    cm_object_property_add_bool(obj, "active-low", &state->active_low);
//    cm_object_property_add_const_str(obj, "on-message", &state->on_message);
//    cm_object_property_add_const_str(obj, "off-message", &state->off_message);
//
//    cm_object_property_set_str(obj, "[display on]\n", "on-message");
//    cm_object_property_set_str(obj, "[display off]\n", "off-message");

#if defined(CONFIG_SDL)

    cm_object_property_add_int16(obj, "x",
            (const int16_t*) &state->display_graphic_context.rectangle.x);
    cm_object_property_add_int16(obj, "y",
            (const int16_t*) &state->display_graphic_context.rectangle.y);
    cm_object_property_add_uint16(obj, "w",
            (const uint16_t*) &state->display_graphic_context.rectangle.w);
    cm_object_property_add_uint16(obj, "h",
            (const uint16_t*) &state->display_graphic_context.rectangle.h);

    cm_object_property_add_uint8(obj, "colour.red", &state->colour.red);
    cm_object_property_add_uint8(obj, "colour.green", &state->colour.green);
    cm_object_property_add_uint8(obj, "colour.blue", &state->colour.blue);

#endif /* defined(CONFIG_SDL) */

    // Create a single incoming irq.
    cm_irq_init_in(DEVICE(obj), gpio_display_irq_handler, IRQ_GPIO_DISPLAY_IN, 1);

    // The connection will be done by the machine.
    // A helper class is gpio_display_connect().

    // Explicitly start with the graphic context cleared.
    cortexm_graphic_display_clear_graphic_context(&(state->display_graphic_context));
}

static void gpio_display_realize_callback(DeviceState *dev, Error **errp)
{
    qemu_log_function_name();

    // Call parent realize().
    if (!cm_device_parent_realize(dev, errp, TYPE_GPIO_DISPLAY)) {
        return;
    }

#if defined(CONFIG_SDL)

    GPIODISPLAYState *state = GPIO_DISPLAY_STATE(dev);

    if (state->board_graphic_context) {

        cortexm_graphic_enqueue_event(GRAPHIC_EVENT_DISPLAY_INIT, state, NULL);
    }

#endif /* defined(CONFIG_SDL) */
}

static void gpio_display_class_init_callback(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = gpio_display_realize_callback;
}

static const TypeInfo gpio_display_type_info = {
    .name = TYPE_GPIO_DISPLAY,
    .parent = TYPE_GPIO_DISPLAY_PARENT,
    .instance_size = sizeof(GPIODISPLAYState),
    .instance_init = gpio_display_instance_init_callback,
    .class_init = gpio_display_class_init_callback,
    .class_size = sizeof(GPIODISPLAYClass)
/**/
};

static void gpio_display_type_init(void)
{
    type_register_static(&gpio_display_type_info);
}

type_init(gpio_display_type_init);

// ----------------------------------------------------------------------------
