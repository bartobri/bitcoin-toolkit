import subprocess

class BTK:

    def __init__(self, command):
        self.command = command
        self.args = []
        self.text = True
        self.input = None

    def set_text(self, text=True):
        self.text = text

    def set_input(self, input):
        self.input = input

    def arg(self, arg, value=None):
        self.args.append(arg)
        if (value):
            self.args.append(value)

    def reset(self, command=None):
        if (command):
            self.__init__(command)
        else:
            self.__init__(self.command)

    def run(self):
        command_str = "bin/btk "
        command_str += f"{self.command} "
        command_str += " ".join(self.args)
        return subprocess.run(command_str, shell=True, capture_output=True, text=self.text, input=self.input)
