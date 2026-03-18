#include <linux/init.h>
#include <linux/module.h>


#include "mouse.h"


static int __init mouse_moduel_init(void) {
    int result;
    result = usb_register(&mouse_driver);
    if (result) {
        printk("Error registering usb");
        return -result;
    }

    return 0;
}

static void __exit mouse_moduel_exit(void) {
    usb_deregister(&mouse_driver);
}

module_init(mouse_moduel_init);
module_exit(mouse_moduel_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jude");
MODULE_DESCRIPTION("Simple Hello Driver");
