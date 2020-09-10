(RISC-V) Proxy Kernel for Education

The RISC-V Proxy Kernel for Education, i.e., pke, is a project that employs the idea
of proxy kernel (running on RISC-V spike emulator) in Education.

Many codes are borrowed from the RISC-V Proxy Kernel and Boot Loader (pk) project, and 
it is the reason for us to maintain all the license files in source code directories 
of pke. Appreciations to the developers of the pk project! 

There are 5 experiments are designed in pke. Each of the experiments has a corresponding
branch (e.g., lab1_small, lab2_small, ..., lab5_small).

If you cloned the project to your local drive by using the command:
$git clone https://github.com/MrShawCode/pke

And you want to switch to another lab (say lab2) after having finished a previous lab
(say lab1), you need to use the following command:
$git checkout -b lab2_small origin/lab2_small

To merge whatever you have done in a previous lab to your current lab, you need to:
$git merge -m "merge from previous lab" <branch_name_of_prev_lab>

<branch_name_of_prev_lab> stands for the short name of the previous lab. For example, 
if your current lab is lab2_small, and the previous lab is lab1_small. The above command
will be:
$git merge -m "merge from lab1" lab1_small


enjoy!

