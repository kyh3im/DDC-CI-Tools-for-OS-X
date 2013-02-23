#include <iostream>
#include <sstream>
#include <string>
#include "ddc.h"
#include "getopt_pp.h"

using namespace GetOpt;

void display_help();
void commands_help();

int main(int argc, char* argv[]) {
	
	bool help_flag;
	bool set_flag;
	bool get_flag;
	std::string new_value;
	std::string display;
	std::string control;
    int new_value_int;
	int display_int;
	int control_int;
	
	
	bool list_displays;
    bool list_commands;
	
	try
	{	
		GetOpt_pp ops(argc, argv);
		
		ops.exceptions(std::ios::failbit);
		
		ops 
		>> OptionPresent('h', "help", help_flag)
        >> OptionPresent('x', "listcommands", list_commands)
		>> OptionPresent('s', "set", set_flag)
		>> OptionPresent('g', "get", get_flag)
		
		>> OptionPresent('l', "listdisplays", list_displays)
		
		>> Option('c', "control", control)
		>> Option('v', "value", new_value, "0")
		>> Option('d', "display", display, "0")
		;
		
		
		if (!ops.options_remain())
		{
            if ((list_commands) ) {
			    commands_help();	
				return 0;
			}
			if ((help_flag) or ((set_flag+get_flag+list_displays) == 0) ) {
			    display_help();	
				return 0;
			}
           
			
			if ((set_flag + get_flag +  list_displays) != 1) {
				std::cerr << "Please specify only one of the following\n";
				std::cerr << "set | get | listcontrols | listdisplays \n";
	            return 1;
			}

			
			if (sscanf(new_value.c_str(), "%d", &new_value_int) == EOF) { std::cerr << "Value needs to be an integer\n"; return 1; }
			if (sscanf(display.c_str(), "%d", &display_int) == EOF) { std::cerr << "Display needs to be an integer\n"; return 1; }
						
			if (set_flag) {
				if (!(ops >>OptionPresent('c',"control")) || !(ops >> OptionPresent('v',"value")) || !(ops >> OptionPresent('d',"display"))) {
					std::cerr << "Control,Value and Display need to be specified\n";
					return 1;
				}
				
                int result;
				struct DDCWriteCommand write_command;
                // hex to dec
                char *string;
                string = strstr(control.c_str(),"0x");
                int dec = 0;
                if (string){
                    int len = (int)strlen(string);
                    len = len - 1;
                    float hex_place = 0.0625;
                    char pos[2];
                    while (len>=2){
                        strncpy(pos,string+(len--),1);
                        if (isalpha(pos[0])){
                            pos[0] = tolower(pos[0]);
                            if (pos[0]<='f') 
                                dec = pos[0]-'a'+10;
                            else hex_place /= 16.0;
                        }
                        write_command.control_id += ((dec>0)?dec:atoi(pos))*(hex_place*=16.0);
                        dec = 0;
                    }
                } else {
                    write_command.control_id = atoi(control.c_str());
                }
                // ends
                write_command.new_value = new_value_int;
				result = ddc_write(display_int, &write_command);
				return !result;
			}
			
			if (get_flag) {
				if (!(ops >>OptionPresent('c',"control"))  || !(ops >> OptionPresent('d',"display"))) {
					std::cerr << "Control and Display need to be specified\n";
					return 1;
				}
				
				int result;
				struct DDCReadCommand read_command;
				read_command.control_id = atoi(control.c_str());
				
				result = ddc_read(display_int, &read_command);
				
				std::cout << "Current Value: " << (int)read_command.response.current_value << "\n";
				std::cout << "Maximum Value: " << (int)read_command.response.max_value << "\n";
				
			}
			
			
			return 0;
		}
		else
		{
			std::cerr << "too many options" << std::endl;
			return 1;
		}
	}
	catch(const GetOptEx& e)
	{
		std::cerr << "Invalid options\n";
		std::cerr << "control should be a string\n";
		std::cerr << "value and display should be integers\n";
		display_help();
	}    
	
	return 0;
}


void display_help() {
	std::cout << "Usage: ddcctrl [options]\n"
    <<"Options:\n"
    <<"-s                         Set command in case you want to control display.\n"
    <<"-g                         Get information from display.\n"
    <<"-d <number>                Required. Specify display number.\n"
    <<"-c <control>               Specify control command. For example 0xe1 controls display power.\n"
    <<"-v <value>                 Specify value which will be send.\n"
    <<"-h                         Display this usage.\n"
    <<"-x                         Display list of commands.\n"
    <<"\ne.g \"$ddctrl -d 0 -s -c 0xe1 -v 1\" For primary display set command 0xe1 with value 1.\n";
}

void commands_help(){
    std::cout << "Note that these commands may not be completely compatible to your display.\n"
    <<"Use these on your own risk.\n"
    <<"\nReset 0x04"
    <<"\nReset brightness and contrast 0x05"
    <<"\nReset geometry 0x06"
    <<"\nReset color 0x08"	
    <<"\nBrightness 0x10" 
    <<"\nContrast 0x12" 
    <<"\nRed gain 0x16"
    <<"\nGreen gain 0x18"
    <<"\nBlue gain 0x1a"
    <<"\nAuto size center 0x1e"	
    <<"\nWidth 0x22"
    <<"\nHeight 0x32"
    <<"\nVertical pos	0x30"
    <<"\nHorizontal pos 0x20"
    <<"\nPincushion amp 0x24"
    <<"\nPincushion phase 0x42"
    <<"\nKeystone balance 0x40"
    <<"\nPincushion balance 0x26"
    <<"\nTop pincushion amp 0x46"
    <<"\nTop pincushion balance 0x48"
    <<"\nBottom pincushion amp 0x4a"
    <<"\nBottom pincushion balance 0x4c"
    <<"\nVertical linearity 0x3a"
    <<"\nVertical linearity balance 0x3c"
    <<"\nHorizontal static convergence 0x28"
    <<"\nVertical static convergence 0x28"
    <<"\nMoire cancel 0x56"
    <<"\nInput source 0x60"
    <<"\nAudio speaker volume 0x62"
    <<"\nRed black level 0x6c"
    <<"\nGreen black level 0x6e"
    <<"\nBlue black level 0x70\n"

    <<"\nSettings 0xb0"        
    <<"\nOn screen display 0xca"
    <<"\nOsd language 0xcc"
    <<"\nDpms 0xd6"
    <<"\nMagic bright 0xdc" 
    <<"\nVcp version 0xdf"
    <<"\nColor preset 0xe0"
    <<"\nPower control 0xe1\n"	
	
    <<"\nTop left screen purity 0xe8"
    <<"\nTop right screen purity 0xe9"
    <<"\nBottom left screen purity 0xe8"
    <<"\nBottom right screen purity 0xeb\n";

}

/*
int main (int argc, char * const argv[]) {
    // insert code here...
    //std::cout << "Hello, World!\n";
	
	int i;
	
	//std::cout << primary_display_id();
	//printf("<#message#>");
	struct DDCWriteCommand write_command;
	
	
	struct DDCReadCommand read_command;
	//p_write_command = &write_command;
	
	
	write_command.control_id = 0x60;
	write_command.new_value = 0x05;
	//write_command.new_value = 0x01;
	
	//printf("<#message#>");
	i = ddc_write(1, &write_command);

	
	//read_command.control_id = 0x10;
	
	//i = ddc_read(1, &read_command); 
	
	//edid_test(0);
	
	//printf("\nend\n");
	
	//printf(read_command.reply_buffer);
	
    //IOI2CConnectRef connection;
	
	//connection = display_connection(0);
	//IOI2CInterfaceClose( connection, kNilOptions );
	
    return 0;
}
 
 */
