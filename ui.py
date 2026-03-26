import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os

# -------- Functions --------

def select_image():
    file_path = filedialog.askopenfilename(
        filetypes=[("Image Files", "*.jpg *.png *.jpeg")]
    )
    if file_path:
        entry.delete(0, tk.END)
        entry.insert(0, file_path)
def send_image():
    image_path = entry.get()

    if not os.path.exists(image_path):
        messagebox.showerror("Error", "Invalid file path")
        return

    try:
        # Only copy if different file
        if os.path.abspath(image_path) != os.path.abspath("capture.jpg"):
            os.system(f"cp '{image_path}' capture.jpg")

        result = subprocess.run(["./pi_client"], capture_output=True, text=True)

        if result.returncode == 0:
            messagebox.showinfo("Success", "Image Sent Successfully!")
        else:
            messagebox.showerror("Error", result.stderr)

    except Exception as e:
        messagebox.showerror("Error", str(e))
def send_image():
    image_path = entry.get()

    if not os.path.exists(image_path):
        messagebox.showerror("Error", "Invalid file path")
        return

    try:
        # Copy selected image as capture.jpg
        os.system(f"cp '{image_path}' capture.jpg")

        # Run client
        result = subprocess.run(["./pi_client"], capture_output=True, text=True)

        if result.returncode == 0:
            messagebox.showinfo("Success", "Image Sent Successfully!")
        else:
            messagebox.showerror("Error", result.stderr)

    except Exception as e:
        messagebox.showerror("Error", str(e))


# -------- UI --------

root = tk.Tk()
root.title("Secure Image Transfer (PQC)")
root.geometry("400x200")

title = tk.Label(root, text="Kyber Secure Image Transfer", font=("Arial", 14))
title.pack(pady=10)

frame = tk.Frame(root)
frame.pack(pady=10)

entry = tk.Entry(frame, width=30)
entry.pack(side=tk.LEFT, padx=5)

browse_btn = tk.Button(frame, text="Browse", command=select_image)
browse_btn.pack(side=tk.LEFT)

send_btn = tk.Button(root, text="Send Image", command=send_image, bg="green", fg="white")
send_btn.pack(pady=20)

root.mainloop()
