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

std::map<std::string,std::vector<std::string> > names2files;

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
	std::vector<std::string>& filenames = names2files[filename];
	const std::string& playfilename = filenames[rand() % filenames.size()];
	std::cout << "Play " << playfilename << '\n';
	std::vector<char> filenameStr(playfilename.begin(), playfilename.end());
	filenameStr.push_back(0);
	char command[] = "mplayer";
	char quiet[] = "-quiet";
	char* const params[4] = { command, quiet, filenameStr.data(), nullptr };
  pid_t child_pid = fork();
  if(child_pid == 0) {
		close(STDERR_FILENO);
		close(STDOUT_FILENO);
		execvp(command, params);
		exit(-1);
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
	std::string font = "Ubuntu " + std::to_string(fontsize);
	int x=0, y=0;
	std::string title("");
	while(configFile)
	{
		std::string line;
		std::getline(configFile, line);
		if(configFile && !title.empty() && !line.empty() && line.find('.') != std::string::npos)
		{
			names2files[title].push_back(line);
			std::cout << "adding " << line << " to " << title << " map.\n" << std::endl;
		}
		else if(configFile && !line.empty())
		{
			if(!title.empty())
			{
				buttons.emplace_back(title);
				Gtk::Button& button = buttons.back();
				button.signal_clicked().connect([title,&data](){ onButton(title, &data); });
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
			title = line;
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
	//window.fullscreen();
	
	app->run(window);
}

