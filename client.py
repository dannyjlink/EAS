from xmlrpc.client import UNSUPPORTED_ENCODING
import json
import serial
import serial.tools.list_ports
import os
import time

# Relative path to preset folder. 
path = os.path.dirname(__file__)

# Parameter functions
def integer_input(prompt, min, max):
    """Takes in user input with prompt & validates based on min and max."""
    while(True):
        result = int(input(prompt))
        if (result < min):
            print("ERR: Invalid Input -- Must be greater than or equal to", min)
            continue
        elif (result > max):
            print("ERR: Invalid Input -- Must be less than or equal to", max)
            continue
        else:
            return result

def float_input(prompt, min, max, description):
    """Takes in user input with prompt & validates based on min and max, returned value is rounded to nearest tenth."""
    while(True):
            result = input(prompt)

            if (result == "D" or result == "d"):
                print(description)
            else: 
                try:
                    result = float(result)
                except:
                    print("ERR: Invalid Input -- Non-numeric input")
                    continue

                if (result < min):
                    print("ERR: Invalid Input -- Must be greater than or equal to", min)

                elif (result > max):
                    print("ERR: Invalid Input -- Must be less than or equal to", max)
                    continue
                else:
                    return round(result, 1)

def yes_no_input(prompt, description = "", S_option = True):
    """Takes in user input with prompt and accepts Y, N, and S, response of S will print description."""
    while(True):
        result = input(prompt)
        if (S_option & (result == "S" or result == "s")):
            print(description)
        elif (result == "Y" or result == "y"):
            return True
        elif (result == "N" or result == "n"):
            return False
        else:
            print("ERR: Invalid Input")
    


def print_dict(obj):
    result = ""
    for key in obj:
        result += key + ": " + str(obj[key]) + "\n"
    return result

# Bluetooth functions
def send_json(bt, response):
    s = json.dumps(response) + "#"
    bt.write(s.encode())
    print("SENT:\n" + s)


def receive(bt):
    s = (bt.read_all()).decode()
    print("Received:", s)
    return s

def get_COM_port():
    raw_ports = serial.tools.list_ports.comports()
    ports = [port.device for port in raw_ports]
    ports.sort()

    options = {}
    prompt = "Choose COM port assigned to HC05:"
    for i in range(len(ports)):
        options[str(i+1)] = ports[i]
        prompt += " [" + str(i+1) + "] " + ports[i]
    choice = integer_input(prompt + "\n", 1, len(options))
    return ports[choice - 1]

# QOL Functions
def save_settings(obj, filename):
    try:
        path = os.path.dirname(__file__) + "\\presets"
        exists = os.path.exists(path)

        if not exists:  # Make folder if it doesn't exist
            print("debug: Making folder")
            os.mkdir(path)
        
        # Save file into folder
        if ".json" not in filename:
            filename += ".json"
        path = "./presets/" + filename
        with open(path, 'w') as fp:
            json.dump(obj, fp)
    except:
        print("Error in saving file.")

def load_settings(filename):
    path = os.path.dirname(__file__) + "\\presets"
    exists = os.path.exists(path)
    if not exists:
        print("ERR: Preset folder not found, nothing to load\n")
        return False
    if ".json" not in filename:
        filename += ".json"
    try:
        f = open(path + "\\" + filename)
        data = json.load(f)
        f.close()
        return data
    except: 
        print("Error loading data from file.")
    


# Prompts and descriptions for user input
prompts = {
    "load":         "Would you like to use a previously made test preset? [Y]es, [N]o\n",
    "default":      "Would you like to use the default settings? [Y]es, [N]o, [S]ee Default Settings\n",
    "test_type":    "What type of test would you like to run?\n[1]Binary Count (default) [2] Walking 1 [3] Walking 0\n",
    "num_monitors": "How many monitors are you testing? [1-5]\n",
    "num_cycles":   "How many test cycles would you like? [1-256]\n",
    "low_dwell":    "How long should the low dwell period be (rounded to nearest 100ms)?\n[0.1-60] seconds (default 10 sec), or [D]escription of low dwell\n",
    "high_dwell":   "How long should the high dwell period be (rounded to nearest 100ms)?\n[0.1-60] seconds (default 30), or [D]escription of high dwell\n",
    "ports":        "Since you're using less than 5 monitors, which ports are monitors connected to?\nThe first DP port is 1 & the HDMI port is 5.\nPut a comma and space between each port number. [E]xample\n",
    "save":         "Do you want to save these settings? [Y]es, [N]o, [S]ee Settings\n"
}

descriptions = {
    "low_dwell":    "Low dwell time is how long a test case is applied.\nSay 3 monitors are connected and the test case is [ON, OFF, ON]. Low dwell time is how long that middle monitor is off before all monitors go back to ON (high dwell).\n",
    "high_dwell":   "High dwell time is how long all monitors are on between test cases.",
    "ports":        "With three monitors connected, one connected to DP1, another connected to DP2, and the last connected to HDMI, the response would be '1, 2, 5'"
}

# Object that will be sent to Arduino. Pre-loaded with default settings
params = {
    "test_type": 1,
    "num_monitors": 5,
    "num_cycles": 5,
    "low_dwell": 10.0,
    "high_dwell": 30.0,
    "ports": [1, 2, 3, 4, 5],
    "COM": "COM4"
}


#   MAIN PROGRAM START   #
print("Welcome to Edge Auto Switch")
print("When prompted for input please respond with whatever is in [brackets] next to the desired option.")
print("Responses are not case sensitive")

default = False
load = yes_no_input(prompts["load"], S_option = False)

if load:
    filename = input("Please enter filename, please do not include the path.")

    loaded = load_settings(filename)
    if (loaded == False):
        load = False
    else:
        params = loaded

default = False
if not load:
    default = yes_no_input(prompts["default"], print_dict(params))  # Determine if operator wants to use defaults


if (not default and not load):  # Begin taking in test parameters if new
    params["COM"]           = get_COM_port()
    params["test_type"]     = integer_input(prompts["test_type"], 1, 3)
    params["num_monitors"]  = integer_input(prompts["num_monitors"], 1, 5)
    params["num_cycles"]    = integer_input(prompts["num_cycles"], 1, 256)
    params["low_dwell"]     = float_input(prompts["low_dwell"], 0.1, 60.0, descriptions["low_dwell"])
    params["high_dwell"]    = float_input(prompts["high_dwell"], 0.1, 60.0, descriptions["high_dwell"])

    if (params["num_monitors"] < 5):    # Ports don't need to be assigned if all ports are used
        while(True):
            raw_ports = input("Since you're using less than 5 monitors, which ports are monitors connected to?\nThe first DP port is 1 & the HDMI port is 5.\nPut a comma and space between each port number. [E]xample\n")
            if (raw_ports == "E" or raw_ports == "e"):
                print(descriptions["ports"])
            else:
                raw_ports = raw_ports.split(", ")
                try:
                    ports = [int(port) for port in raw_ports]
                except:
                    if (len(raw_ports) == 1):
                        print("ERR: Most likely formatting error in input. Ensure each port number is separated by a comma and space. See [E]xample to see correct formatting.")
                    continue
                
                # Validate port assignments
                # Check that number of ports is equal to number of monitors
                if (len(ports) != params["num_monitors"]):
                    print("Number of monitors does not match number of ports. There should be", params["num_monitors"], "ports input")
                    continue
                # Check that there are no duplicates
                duplicate = False
                out_of_range = False
                for i in range(len(ports)):
                    if (ports[i] < 1 or ports[i] > 5):
                        out_of_range = True
                    for j in range(len(ports)):
                        if (ports[i] == ports[j] and i != j):
                            duplicate = True
                
                if (duplicate):
                    print("Duplicate port. There can only be one instance of any port.")
                    continue
                if (out_of_range):
                    print("Invalid port number. Ports are numbered from 1 to 5")
                    continue

                # Put ports in ascending order and set as test parameters
                ports.sort()
                params["ports"] = ports
                break


if (not load):
    save = yes_no_input(prompts["save"], print_dict(params))
    if (save):
        filename = input("Enter desired filename.\n")
        save_settings(params, filename)

print("Parameter input done:")
print(print_dict(params))
print()

while True:
    d = input("Press enter to begin")
    if (d == ""):
        bt = serial.Serial(params["COM"], 9600)
        print("Connected")
        bt.flushInput()
        print("Flushed")
        send_json(bt, params)
        #time.sleep(5)
        #bt.close()
        #os._exit(0)
        