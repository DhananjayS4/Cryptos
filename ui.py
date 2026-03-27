import customtkinter as ctk
import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os
import shutil
from PIL import Image

# Set appearance mode and color theme
ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Secure Image Transfer (Kyber PQC)")
        self.geometry("750x500")
        
        # Configure grid layout
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # -----------------
        # Sidebar Frame
        # -----------------
        self.sidebar_frame = ctk.CTkFrame(self, width=200, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(3, weight=1)

        self.logo_label = ctk.CTkLabel(
            self.sidebar_frame, 
            text="Kyber\nSecure Transfer", 
            font=ctk.CTkFont(size=22, weight="bold")
        )
        self.logo_label.grid(row=0, column=0, padx=20, pady=(30, 10))

        self.status_label = ctk.CTkLabel(
            self.sidebar_frame, 
            text="Status: Ready", 
            text_color="gray",
            font=ctk.CTkFont(size=14, weight="bold")
        )
        self.status_label.grid(row=1, column=0, padx=20, pady=5)

        # Theme Switcher
        self.appearance_mode_label = ctk.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=4, column=0, padx=20, pady=(10, 0))
        
        self.appearance_mode_menu = ctk.CTkOptionMenu(
            self.sidebar_frame, 
            values=["Light", "Dark", "System"],
            command=self.change_appearance_mode
        )
        self.appearance_mode_menu.grid(row=5, column=0, padx=20, pady=(10, 20))
        self.appearance_mode_menu.set("Dark")

        # -----------------
        # Main Frame
        # -----------------
        self.main_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.main_frame.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)
        self.main_frame.grid_columnconfigure(0, weight=1)
        self.main_frame.grid_rowconfigure(1, weight=1)

        self.header_label = ctk.CTkLabel(
            self.main_frame, 
            text="Post-Quantum Image Upload", 
            font=ctk.CTkFont(size=28, weight="bold")
        )
        self.header_label.grid(row=0, column=0, pady=(10, 20), sticky="n")

        # Image Preview Area
        self.preview_frame = ctk.CTkFrame(self.main_frame, corner_radius=15)
        self.preview_frame.grid(row=1, column=0, sticky="nsew", pady=(0, 20))
        self.preview_frame.grid_propagate(False)

        self.preview_label = ctk.CTkLabel(
            self.preview_frame, 
            text="No Image Selected\nClick 'Browse' to choose a file.", 
            text_color="gray",
            font=ctk.CTkFont(size=14, slant="italic")
        )
        self.preview_label.place(relx=0.5, rely=0.5, anchor="center")

        # Input Area (Entry + Browse)
        self.input_frame = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.input_frame.grid(row=2, column=0, pady=10, sticky="ew")
        self.input_frame.grid_columnconfigure(0, weight=1)

        self.entry = ctk.CTkEntry(
            self.input_frame, 
            placeholder_text="Path to image file...", 
            height=45, 
            font=ctk.CTkFont(size=14)
        )
        self.entry.grid(row=0, column=0, sticky="ew", padx=(0, 15))

        self.browse_btn = ctk.CTkButton(
            self.input_frame, 
            text="Browse", 
            width=100, 
            height=45, 
            font=ctk.CTkFont(size=14, weight="bold"),
            command=self.select_image
        )
        self.browse_btn.grid(row=0, column=1)

        # Send Area (Progress + Button)
        self.progressbar = ctk.CTkProgressBar(self.main_frame, mode="indeterminate", width=400)
        self.progressbar.grid(row=3, column=0, pady=(10, 0))
        self.progressbar.set(0)
        self.progressbar.grid_remove()

        self.send_btn = ctk.CTkButton(
            self.main_frame, 
            text="Securely Send Image", 
            height=50, 
            font=ctk.CTkFont(size=16, weight="bold"), 
            fg_color="#2FA572", 
            hover_color="#106A43", 
            command=self.send_image
        )
        self.send_btn.grid(row=4, column=0, pady=(20, 10), sticky="ew", padx=60)

    def change_appearance_mode(self, new_mode: str):
        ctk.set_appearance_mode(new_mode)

    def select_image(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("Image Files", "*.jpg *.png *.jpeg")]
        )
        if file_path:
            self.entry.delete(0, tk.END)
            self.entry.insert(0, file_path)
            self.load_image_preview(file_path)

    def load_image_preview(self, path):
        try:
            # We enforce a dynamic resize logic to fit into the frame roughly
            img = Image.open(path)
            # Create a larger CTkImage to serve as the preview 
            ctk_img = ctk.CTkImage(light_image=img, dark_image=img, size=(350, 220))
            self.preview_label.configure(image=ctk_img, text="")
            self.preview_label.image = ctk_img
        except Exception as e:
            self.preview_label.configure(text="Preview not available", image="")

    def send_image(self):
        image_path = self.entry.get()

        if not os.path.exists(image_path):
            messagebox.showerror("Error", "Invalid file path selected")
            return

        # UI Updates
        self.status_label.configure(text="Status: Sending...", text_color="#2FA572")
        self.send_btn.configure(state="disabled", text="Encrypting and Sending...")
        self.progressbar.grid()
        self.progressbar.start()

        # Run process slightly delayed so UI can update 
        self.after(100, self.process_image, image_path)

    def process_image(self, image_path):
        try:
            dest_path = "capture.jpg"
            if os.path.abspath(image_path) != os.path.abspath(dest_path):
                shutil.copy(image_path, dest_path)

            executable = "./pi_client"
            result = subprocess.run([executable], capture_output=True, text=True)

            if result.returncode == 0:
                self.status_label.configure(text="Status: Success", text_color="green")
                messagebox.showinfo("Success", "Image sent successfully via Kyber PQC!")
            else:
                self.status_label.configure(text="Status: Failed", text_color="red")
                messagebox.showerror("Transfer Error", result.stderr if result.stderr else "Unknown runtime error")

        except Exception as e:
            self.status_label.configure(text="Status: Error", text_color="red")
            messagebox.showerror("Critical Error", str(e))
        
        finally:
            self.progressbar.stop()
            self.progressbar.grid_remove()
            self.send_btn.configure(state="normal", text="Securely Send Image")

if __name__ == "__main__":
    app = App()
    app.mainloop()
