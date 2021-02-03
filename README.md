已完成该项目的所有实验
**(RISC-V) Proxy Kernel for Education**

The RISC-V Proxy Kernel for Education (PKE for short) is a project that employs the idea
of proxy kernel (i.e., very thin layer operating system) in Education of Operating Systems.

Many codes of PKE are borrowed from the [RISC-V Proxy Kernel and Boot Loader](https://github.com/riscv/riscv-pk) (PK) project, and it is the reason for us to maintain all the license files in source code directories of PKE. Appreciations to the developers of the PK project! 

There are 5 experiments are designed in PKE. Each of the experiments has a corresponding branch (e.g., lab1_small, lab2_small, ..., lab5_small).

If you cloned the project to your local drive by using the command:

`$git clone https://github.com/MrShawCode/pke`

And you want to switch to another lab (say lab2) after having finished a previous lab
(say lab1), you need to use the following command:

`$git checkout -b lab2_small origin/lab2_small`

To merge whatever you have done in a previous lab to your current lab, you need to:

`$git merge -m "merge from previous lab" <branch_name_of_prev_lab>`

<branch_name_of_prev_lab> stands for the short name of the previous lab. For example, 
if your current lab is lab2_small, and the previous lab is lab1_small. The above command
will be:

`$git merge -m "merge from lab1" lab1_small`



**Documents in CHINESE can be found [here](https://gitee.com/syivester/pke-doc).**



enjoy!

