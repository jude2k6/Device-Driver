import tkinter as tk # the big GUI library
import threading # for multithreading so the mouse can be read in the background
import struct # for packing/unpacking binary data
import fcntl # for making ioctl calls to the driver
import os # for opening and reading the device file

DEVICE_PATH = "/dev/MOUSE"
BUFFER_SIZE = 64

# Mirrors the _IOW/_IOR macros from shared_ioctl.h
def _IOC(direction, type_, nr, size):
    return (direction << 30) | (type_ << 8) | nr | (size << 16)

MOUSE_SET_LEDS = _IOC(1, 42, 1, 4)  # _IOW(42, 1, led_packet)
MOUSE_SET_DPI  = _IOC(1, 42, 3, 4)  # _IOW(42, 3, int)

fd = os.open(DEVICE_PATH, os.O_RDWR)

root = tk.Tk()
root.title("Mouse Driver")

# Labels for live data
x_var   = tk.StringVar(value="X: 0")
y_var   = tk.StringVar(value="Y: 0")
btn_var = tk.StringVar(value="Buttons: none")

tk.Label(root, textvariable=x_var).pack()
tk.Label(root, textvariable=y_var).pack()
tk.Label(root, textvariable=btn_var).pack()

# DPI control
tk.Label(root, text="DPI:").pack()
dpi_entry = tk.Entry(root)
dpi_entry.insert(0, "800")
dpi_entry.pack()

def set_dpi():
    try:
        dpi = int(dpi_entry.get())
        fcntl.ioctl(fd, MOUSE_SET_DPI, struct.pack('i', dpi))
    except OSError as e:
        print(f"DPI error: {e}")

tk.Button(root, text="Set DPI", command=set_dpi).pack()

# LED control
tk.Label(root, text="LED (R G B):").pack()
r_entry = tk.Entry(root, width=4)
g_entry = tk.Entry(root, width=4)
b_entry = tk.Entry(root, width=4)
r_entry.insert(0, "255")
g_entry.insert(0, "0")
b_entry.insert(0, "0")
r_entry.pack()
g_entry.pack()
b_entry.pack()

def set_led():
    try:
        r, g, b = int(r_entry.get()), int(g_entry.get()), int(b_entry.get())
        fcntl.ioctl(fd, MOUSE_SET_LEDS, struct.pack('BBBB', 0, r, g, b))
    except OSError as e:
        print(f"LED error: {e}")

tk.Button(root, text="Set LED", command=set_led).pack()

# Background thread - blocks on read() until driver sends data
def read_loop():
    while True:
        data = os.read(fd, BUFFER_SIZE)
        if len(data) >= 7:
            buttons = data[1]  # data[0] is HID report ID on my logitech bluetooth mouse
            x = struct.unpack_from('<h', data, 2)[0]
            y = struct.unpack_from('<h', data, 4)[0]
            btns = []
            if buttons & 0x01: btns.append("Left")
            if buttons & 0x02: btns.append("Right")
            if buttons & 0x04: btns.append("Middle")
            root.after(0, x_var.set, f"X: {x}")
            root.after(0, y_var.set, f"Y: {y}")
            root.after(0, btn_var.set, f"Buttons: {', '.join(btns) or 'none'}")

threading.Thread(target=read_loop, daemon=True).start()

root.mainloop()
os.close(fd)
