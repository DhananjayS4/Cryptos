import customtkinter as ctk
import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os
import shutil

# Set appearance mode and color theme
ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Secure Image Transfer (PQC)")
        self.geometry("500x250")

        # Create main frame
        self.main_frame = ctk.CTkFrame(self, corner_radius=15)
        self.main_frame.pack(pady=20, padx=20, fill="both", expand=True)

        # Title Label
        self.title_label = ctk.CTkLabel(
            self.main_frame,
            text="Kyber Secure Image Transfer",
            font=ctk.CTkFont(size=20, weight="bold")
        )
        self.title_label.pack(pady=(20, 15))

        # Input Frame (for Entry and Browse btn)
        self.input_frame = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.input_frame.pack(pady=10, fill="x", padx=20)

        self.entry = ctk.CTkEntry(
            self.input_frame,
            placeholder_text="Select an image file...",
            width=280,
            height=35
        )
        self.entry.pack(side="left", padx=(0, 10))

        self.browse_btn = ctk.CTkButton(
            self.input_frame,
            text="Browse",
            width=80,
            height=35,
            command=self.select_image
        )
        self.browse_btn.pack(side="left")

        # Send Button
        self.send_btn = ctk.CTkButton(
            self.main_frame,
            text="Send Image",
            font=ctk.CTkFont(size=14, weight="bold"),
            height=40,
            command=self.send_image
        )
        self.send_btn.pack(pady=(15, 20))

    def select_image(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("Image Files", "*.jpg *.png *.jpeg")]
        )
        if file_path:
            self.entry.delete(0, tk.END)
            self.entry.insert(0, file_path)

    def send_image(self):
        image_path = self.entry.get()

        if not os.path.exists(image_path):
            messagebox.showerror("Error", "Invalid file path")
            return

        try:
            # Safely copy image using cross-platform shutil instead of os.system("cp")
            dest_path = "capture.jpg"
            if os.path.abspath(image_path) != os.path.abspath(dest_path):
                shutil.copy(image_path, dest_path)

            # Run client
            executable = "./pi_client"
            result = subprocess.run([executable], capture_output=True, text=True)

            if result.returncode == 0:
                messagebox.showinfo("Success", "Image Sent Successfully!")
            else:
                messagebox.showerror("Error", result.stderr if result.stderr else "Unknown error occurred")

        except Exception as e:
            messagebox.showerror("Error", str(e))

if __name__ == "__main__":
    app = App()
    app.mainloop()
