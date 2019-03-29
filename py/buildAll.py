import sys
from BLibUtil import*
def BuildAir(build_info):
    additional_options = ""
    for compiler_info in build_info.compilers:
        BuildAProject("Air", "..", build_info, compiler_info, False, additional_options)
        
if __name__ == "__main__":
    BuildAir(BuildInfo.FromArgv(sys.argv))