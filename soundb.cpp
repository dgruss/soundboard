#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/window.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>

struct Data {
	Gtk::Window window;
	Gtk::Button stopButton;
	std::vector<pid_t> pids;
};

void onButton(const std::string& filename, Data* data)
{
	std::vector<pid_t> newPids;
	for(pid_t pid : data->pids)
	{
		int status;
		if(waitpid(pid, &status, WNOHANG) != -1)
		{
			if(!WIFEXITED(status))
				newPids.emplace_back(pid);
		}
	}
	data->pids = std::move(newPids);
	std::cout << "Number of pids: " << data->pids.size() << '\n';
	
	std::cout << "Play " << filename << '\n';
	std::vector<char> filenameStr(filename.begin(), filename.end());
	filenameStr.push_back(0);
	char command[] = "mplayer";
	char* const params[3] = { command, filenameStr.data(), nullptr };
  pid_t child_pid = fork();
  if(child_pid == 0) {
		execvp(command, params);
		std::cout << "execvp failed.\n";
	}
	else {
		data->pids.push_back(child_pid);
		data->stopButton.set_sensitive(true);
	}
}

void onStop(Data* data)
{
	for(pid_t pid : data->pids)
	{
		kill(pid, SIGKILL);
	}
	data->pids.clear();
	//if(tpid != child_pid) process_terminated(tpid);
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cerr << "Syntax: soundb <configfile>\n";
		return -1;
	}
	auto app =
    Gtk::Application::create("");
	Data data;
	Gtk::Window& window = data.window;
	Gtk::Grid grid;
	std::vector<Gtk::Button> buttons;
	
	std::ifstream configFile(argv[1]);
	int cols, rows, fontsize;
	configFile >> cols >> rows >> fontsize;
	if(fontsize < 8) fontsize = 8;
	std::string font = "Eras Demi ITC " + std::to_string(fontsize);
	int x=0, y=0;
	while(configFile)
	{
		std::string line;
		std::getline(configFile, line);
		if(configFile && !line.empty())
		{
			std::string title = line;
			std::getline(configFile, line);
			if(configFile && !line.empty())
			{
				buttons.emplace_back(title);
				Gtk::Button& button = buttons.back();
				button.signal_clicked().connect([line,&data](){ onButton(line, &data); });
				button.set_hexpand(true);
				button.set_vexpand(true);
				button.override_font(Pango::FontDescription(font));
				grid.attach(button, x, y, 1, 1);
				++x;
				if(x == cols)
				{
					x = 0;
					y++;
				}
			}
			else {
				throw std::runtime_error("Error in config file: after '" + title + "'");
			}
		}
	}
	
	data.stopButton.set_label("Stop");
	data.stopButton.override_font(Pango::FontDescription(font));
	data.stopButton.set_sensitive(false);
	data.stopButton.set_hexpand(true);
	data.stopButton.set_vexpand(true);
	data.stopButton.signal_clicked().connect([&data](){ onStop(&data); });
	
	grid.attach(data.stopButton, x, y, 1, 1);
	grid.set_hexpand(true);
	grid.set_vexpand(true);
	
	window.add(grid);
	window.show_all_children();
	window.fullscreen();
	
	app->run(window);
}

