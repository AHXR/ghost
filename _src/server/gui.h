/*
	@title
		ghost
	@author
		AHXR (https://github.com/AHXR)
	@copyright
		2017

	ghost is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ghost is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ghost.  If not, see <http://www.gnu.org/licenses/>.
*/
//=======================================================
#pragma once

namespace server {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for gui
	/// </summary>
	public ref class gui : public System::Windows::Forms::Form
	{
	public:
		gui(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~gui()
		{
			if (components)
			{
				delete components;
			}
		}
	public: static System::Windows::Forms::NotifyIcon^  taskbarIcon;
	protected:

	protected:

	protected:

	protected:


	protected:
	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(gui::typeid));
			this->taskbarIcon = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->SuspendLayout();
			// 
			// taskbarIcon
			// 
			this->taskbarIcon->BalloonTipIcon = System::Windows::Forms::ToolTipIcon::Info;
			this->taskbarIcon->BalloonTipText = L"New Client";
			this->taskbarIcon->BalloonTipTitle = L"ghost";
			this->taskbarIcon->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"taskbarIcon.Icon")));
			this->taskbarIcon->Text = L"ghost";
			this->taskbarIcon->Visible = true;
			this->taskbarIcon->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &gui::taskbarIcon_MouseDoubleClick);
			// 
			// gui
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(164, 19);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"gui";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->Text = L"#ghost (github.com/AHXR)";
			this->WindowState = System::Windows::Forms::FormWindowState::Minimized;
			this->Load += gcnew System::EventHandler(this, &gui::gui_Load);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void gui_Load(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void taskbarIcon_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
		if (b_hidden) {
			SHOW_CONSOLE();
			freopen("conin$", "r", stdin);
			freopen("conout$", "w", stdout);
			freopen("conout$", "w", stderr);
			HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
			ahxrlogger_handle(hstdout);
			
			WARNING("Refreshing clients... Please wait.");
			refreshClients();

			SHOW_GHOST();
		}
			
	}
	};
}
