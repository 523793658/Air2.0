import sys, os, multiprocessing, subprocess, shutil, platform

def LogWarning(message):
    print("W %s" % message)
    
def LogError(message):
    print("[E] %s" % message)
    if 0 == sys.platform.find("win"):
        pause_cmd = "pause"
    else:
        pause_cmd = "read"
    subprocess.call(pause_cmd, shell = True)
    sys.exit(1)

class CompilerInfo:
	def __init__(self, arch, gen_name, compiler_root, vcvarsall_path = "", vcvarsall_options = ""):
		self.arch = arch
		self.generator = gen_name
		self.compiler_root = compiler_root
		self.vcvarsall_path = vcvarsall_path
		self.vcvarsall_options = vcvarsall_options

class BuildInfo:
    @classmethod
    def fromArgv(BuildInfo, argv, base = 0):
        project = ""
        compiler = ""
        archs = ""
        cfg = ""
        target = ""
        
        argc = len(argv)
        return BuildInfo(project, compiler, archs, cfg, target)
        
    def __init__(self, project, compiler, archs, cfg, target):
        from CfgBuild import ActivedCfgBuild
        cfg_build = ActivedCfgBuild()

        try:
            cfg_build.cmake_path
        except:
            cfg_build.cmake_path = "auto"
        
        if len(project) > 0:
            cfg_build.project = project
        else:
            try:
                cfg_build.project
            except:
                cfg_build.project = "auto"
        
        if len(compiler) > 0 and (compiler.lower() != "clean"):
            cfg_build.compiler = compiler
        else:
            try:
                cfg_build.compiler
            except:
                cfg_build.compiler = "auto"
        
        if len(archs) > 0:
            cfg_build.arch = archs
        else:
            try:
                cfg_build.arch
            except:
                cfg_build.arch = ("x64", )
        
        if len(cfg) > 0:
            cfg_build.config = cfg
        else:
            try:
                cfg_build.config
            except:
                cfg_build.config = ("Debug", "RelWithDebInfo")

        if len(target) > 0:
            cfg_build.target = target
        else:
            try:
                cfg_build.target
            except:
                cfg_build.target = "auto"
                
        try:
            cfg_build.shader_platform_name
        except:
            cfg_build.shader_platform_name = "auto"
            
        try:
            cfg_build.gles_include_dir
        except:
            cfg_build.gles_include_dir = "auto"
            
        try:
            cfg_build.libovr_path
        except:
            cfg_build.libovr_path = "auto"
            
        env = os.environ
        host_platform = sys.platform
        if 0 == host_platform.find("win"):
            host_platform = "win"
        
        if "auto" == cfg_build.target:
            target_platform = host_platform
        else:
            target_platform = cfg_build.target
            if 0 == target_platform.find("android"):
                if not "ANDROID_NDK" in env:
                    LogError("You must define an ANDROID_NDK environment variable to your location of NDK.\n")
                
                space_place = target_platform.find(' ')
                if space_place != -1:
                    android_ver = target_platform[space_place + 1:]
                    target_platform = target_platform[0:space_place]
                    if "8.1" == android_ver:
                        target_api_level = 27
                    elif "8.0" == android_ver:
                        target_api_level = 26
                    elif "7.1" == android_ver:
                        target_api_level = 25
                    elif "7.0" == android_ver:
                        target_api_level = 24
                    elif "6.0" == android_ver:
                        target_api_level = 23
                    elif "5.1" == android_ver:
                        target_api_level = 22
                    else:
                        LogError("Unsupported Android Version.\n")
                else:
                    target_api_level = 22
                self.target_api_level = target_api_level
        if ("android" == target_platform) or ("ios" == target_platform):
            prefer_static = True
        else:
            prefer_static = False
            
        shader_platform_name = cfg_build.shader_platform_name
        if 0 == len(shader_platform_name):
            shader_platform_name = "auto"
        
        self.host_platform = host_platform
        self.target_platform = target_platform
        self.shader_platform_name = shader_platform_name
        self.prefer_static = prefer_static
        self.is_clean = ("clean" == compiler)
        
        if self.host_platform == "win":
            self.where_cmd = "where"
            self.sep = "\r\n"
        else:
            self.where_cmd = "which"
            self.sep = "\n"
            
        self.cmake_path = cfg_build.cmake_path
        if self.cmake_path == "auto":
            self.cmake_path = self.findCMake()
        self.cmake_ver = self.retrieveCMakeVersion()
        if self.cmake_ver < 39:
            LogError("CMake 3.9+ is required.")
        
        self.is_windows_desktop = False
        self.is_windows = False
        self.is_android = False
        
        if "win" == target_platform:
            self.is_windows = True
            self.is_windows_desktop = True
        elif "android" == target_platform:
            self.is_android = True
        
        self.is_dev_platform = self.is_windows_desktop
        
        if len(cfg_build.project) > 0:
            project_type = cfg_build.project
        else:
            project_type = ""
        if ("" == compiler) or self.is_clean:
            compiler = ""
            if ("auto" == cfg_build.project) and ("auto" == cfg_build.compiler):
                if 0 == target_platform.find("win"):
                    progam_files_folder = self.findProgramFilesFolder()
                    
                    if len(self.findVS2019Folder(progam_files_folder)) > 0:
                        project_type = "vs2019"
                        compiler = "vc142"
                    elif len(self.findVS2017Folder(progam_files_folder))> 0:
                        project_type = "vs2017"
                        compiler = "vc141"
                    elif ("VS140COMNTOOLS" in env) or os.path.exists(progam_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\VCVARSALL.BAT"):
                        project_type = "vs2015"
                        compiler = "vc140"
                    elif len(self.findClang())!= 0:
                        project_type = "make"
                        compiler = "clang"
                    elif len(self.findGCC()) != 0:
                        project_type = "make"
                        compiler = "mingw"
                elif ("android" == target_platform):
                    project_type = "make"
                    compiler = "clang"
                else:
                    LogError("Unsupported target platform.\n")
            else:
                if cfg_build.project != "auto":
                    project_type = cfg_build.project
                if cfg_build.compiler != "auto":
                    compiler = cfg_build.compiler
        if(project_type != "") and (compiler == ""):
            if project_type == "vs2019":
                compiler = "vc142"
            elif project_type == "vs2017":
                compiler = "vc141"
            elif project_type == "vs2015":
                compiler = "vc140"
            elif project_type == "xcode":
                compiler = "clang"
        
        if 0 == target_platform.find("win"):
            program_files_folder = self.findProgramFilesFolder()
            if "vc142" == compiler:
                if project_type == "vs2019":
                    print("5555555555555")
                    try_folder = self.findVS2019Folder(program_files_folder)
                    if len(try_folder) > 0:
                        print("666666666666")
                        compiler_root = try_folder
                        vcvarsall_path = "VCVARSALL.BAT"
                        vcvarsall_options = "amd64_arm64 -vcvars_ver=14.2"
                    else:
                        LogError("Could not find vc142 compiler toolset for vs2019.\n")
                else:
                    LogError("Could NOT find vc142 compiler.\n")
            elif "vc141" == compiler:
                if project_type == "vs2019":
                    try_folder == self.findVS2019Folder(program_files_folder)
                    if len(try_folder) > 0:
                        compiler_root = try_folder
                        vcvarsall_path = "VCVARSALL.BAT"
                        vcvarsall_options = "-vcvars_ver=14.1"
                    else:
                        LogError("Could not find vc141 compiler toolset for vs2019.\n")
                else:
                    try_folder = self.findVS2017Folder(program_files_folder)
                    if len(try_folder) > 0:
                        compiler_root = try_folder
                        vcvarsall_path = "VCVARSALL.BAT"
                    else:
                        LogError("Could NOT find vc141 compiler.\n")
                    vcvarsall_options = ""
            elif "vc140" == compiler:
                if project_type == "vs2019":
                    try_folder = self.findVS2019Folder(program_files_folder)
                    if len(try_folder) > 0:
                        compiler_root = try_folder
                        vcvarsall_path = "VCVARSALL.BAT"
                        vcvarsall_options = "-vcvars_ver=14.0"
                    else:
                        LogError("Could NOT find vc140 compiler toolset for VS2017.\n")
                elif project_type == "vs2017":
                    try_folder = self.FindVS2017Folder(program_files_folder)
                    if len(try_folder) > 0:
                        compiler_root = try_folder
                        vcvarsall_path = "VCVARSALL.BAT"
                        vcvarsall_options = "-vcvars_ver=14.0"
                    else:
                        LogError("Could NOT find vc140 compiler toolset for VS2017.\n")
                else:
                    if "VS140COMNTOOLS" in env:
                        compiler_root = env["VS140COMNTOOLS"] + "..\\..\\VC\\bin\\"
                        vcvarsall_path = "..\\VCVARSALL.BAT"
                    else:
                        try_folder = program_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\bin\\"
                        try_vcvarsall = "..\\VCVARSALL.BAT"
                        if os.path.exists(try_folder + try_vcvarsall):
                            compiler_root = try_folder
                            vcvarsall_path = try_vcvarsall
                        else:
                            LogError("Could NOT find vc140 compiler.\n")
                    vcvarsall_options = ""
            elif "clang" == compiler:
                clang_loc = self.findClang()
                compiler_root = clang_loc[0:clang_loc.rfind("\\clang++")+1]
            elif "mingw" == compiler:
                gcc_loc = self.findGCC()
                compiler_root = gcc_loc[0:gcc_loc.rfind("\\g++")+1]
        else:
            compiler_root = ""
            
        if "" == project_type:
            if "vc142" == compiler:
                project_type = "vs2019"
            elif "vc141" == compiler:
                project_type = "vs2017"
            elif "vc140" == compiler:
                project_type = "vs2015"
            elif ("clang" == compiler) and (("darwin" == target_platform) or ("ios" == target_platform)):
                project_type = "xcode"
            else:
                project_type = "make"
        if "" == archs:
            archs = cfg_build.arch
            if "" == archs:
                archs = ("x64", )
        
        if "" == cfg:
            cfg = cfg_build.config
            if "" == cfg:
                cfg = ("Debug", "RelWithDebInfo")
                
        multi_config = False
        compilers = []
        if "vs2019" == project_type:
            print("22222222222--%s" % compiler)
            self.vs_version = 16
            if "vc142" == compiler:
                print("3333333333")
                compiler_name = "vc"
                compiler_version = 142
            elif "vc141" == compiler:
                compiler_name = "vc"
                compiler_version = 141
            elif "vc140" == compiler:
                compiler_name = "vc"
                compiler_version = 140
            else:
                LogError("Wrong combination of project %s and compiler %s.\n" %(project_type, compiler))
            multi_config = True
            for arch in archs:
                print("4444444444--%s" % vcvarsall_options)
                compilers.append(CompilerInfo(arch, "Visual Studio 16", compiler_root, vcvarsall_path, vcvarsall_options))
        elif "vs2017" == project_type:
            self.vs_version = 15
            if "vc141" == compiler:
                compiler_name = "vc"
                compiler_version = 141
            elif "vc140" == compiler:
                compiler_name = "vc"
                compiler_version = 140
            else:
                LogError("Wrong combination of project %s and compiler %s.\n" %(project_type, compiler))
            multi_config = True
            for arch in archs:
                compilers.append(CompilerInfo(arch, "Visual Studio 15", compiler_root, vcvarsall_path, vcvarsall_options))
        elif "vs2015" == project_type:
            self.vs_version = 14
            if "vc140" == compiler:
                compiler_name = "vc"
                compiler_version = 140
            else:
                 LogError("Wrong combination of project %s and compiler %s.\n" %(project_type, compiler))
            multi_config = True
            for arch in archs:
                compilers.append(CompilerInfo(arch, "Visual Studio 14", compiler_root, vcvarsall_path, vcvarsall_options))
        elif "xcode" == project_type:
            if "clang" == compiler:
                compiler_name = "clang"
                compiler_version = self.retrieveClangVersion()
                gen_name = "Xcode"
                multi_config = True
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root))
            else:
                LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
        elif ("make" == project_type) or ("ninja" == project_type):
            if "ninja" == project_type:
                gen_name = "Ninja"
            else:
                if "win" == host_platform:
                    gen_name = "MinGW Makefiles"
                else:
                    gen_name = "Unix Makefiles"
            if "clang" == compiler:
                compiler_name = "clang"
                compiler_version = self.retrieveClangVersion()
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root))
            elif "mingw" == compiler:
                compiler_name = "mgw"
                compiler_version = self.retrieveGCCVersion()
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root))
            elif "gcc" == compiler:
                compiler_name = "gcc"
                compiler_version = self.retrieveGCCVersion()
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root))
            elif "vc142" == compiler:
                compiler_name = "vc"
                compiler_version = 142
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root, vcvarsall_path, vcvarsall_options))
            elif "vc141" == compiler:
                compiler_name = "vc"
                compiler_version = 141
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root, vcvarsall_path, vcvarsall_options))
                    
            elif "vc140" == compiler:
                compiler_name = "vc"
                compiler_version = 140
                for arch in archs:
                    compilers.append(CompilerInfo(arch, gen_name, compiler_root, vcvarsall_path, vcvarsall_options))
            else:
                LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
        else:
            compiler_name = ""
            compiler_version = 0
            LogError("Unsupported compiler.\n")
        
        if 0 == project_type.find("vs"):
            self.proj_ext_name = "vcxproj"
        
        self.project_type = project_type
        self.compiler_name = compiler_name
        self.compiler_version = compiler_version
        self.multi_config = multi_config
        self.compilers = compilers
        self.cfg = cfg
        self.gles_include_dir = cfg_build.gles_include_dir
        self.libovr_path = cfg_build.libovr_path
        self.jobs = multiprocessing.cpu_count()
        self.displayInfo();
                    
    def displayInfo(self):
        print("Build information:")
        print("\tCMake path: %s" % self.cmake_path)
        print("\tCMake version: %s" % self.cmake_ver)
        print("\tHost platform: %s" % self.host_platform)
        print("\tTarget platform: %s" % self.target_platform)
        if self.is_android:
            print("\tTarget API level: %d" % self.target_api_level)
        print("\tCPU count: %d" % self.jobs)
        print("\tPrefer static library: %s" % self.prefer_static)
        print("\tShader platform: %s" % self.shader_platform_name)
        print("\tIs dev platform: %s" % self.is_dev_platform)
        print("\tProject type: %s" % self.project_type)
        print("\tCompiler: %s%d" % (self.compiler_name, self.compiler_version))
        archs = ""
        for i in range(0, len(self.compilers)):
            archs += self.compilers[i].arch
            if i != len(self.compilers) - 1:
                archs += ", "
        print("\tArchitectures: %s" % archs)
        cfgs = ""
        for i in range(0, len(self.cfg)):
            cfgs += self.cfg[i]
            if i != len(self.cfg) - 1:
                cfgs += ", "
        print("\tConfigures: %s" % cfgs)
        print("\tGLES SDK include path: %s" % self.gles_include_dir)
        if self.is_windows_desktop:
            print("\tOculus LibOVR path: %s" % self.libovr_path)
        print("")
        sys.stdout.flush()
                    
    def findVS2019Folder(self, progam_files_folder):
        return self.findVS2017PlusFolder(progam_files_folder, 16, "2019")
        
    def findVS2017Folder(self, progam_files_folder):
        return self.findVS2017PlusFolder(progam_files_folder, 15, "2019")
    
    def findVS2017PlusFolder(self, progam_files_folder, vs_version, vs_name):
        try_vswhere_location = progam_files_folder + "\\Microsoft Visual Studio\\Installer\\vswhere.exe"
        if os.path.exists(try_vswhere_location):
            vs_location = subprocess.check_output([try_vswhere_location, 
                "-products", "*",
                "-latest",
                "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                "-property", "installationPath",
                "-version", "[%d.0,%d.0)" % (vs_version, vs_version+1),
                "-prerelease"]).decode().split("\r\n")[0]
            
            try_folder = vs_location + "\\VC\\Auxiliary\\Build\\"
            try_vcvarsall = "VCVARSALL.BAT"
            if os.path.exists(try_folder + try_vcvarsall):
                return try_folder
        else:
            names = ("Preview", vs_name)
            skus = ("Community", "Professional", "Enterprise", "BuildTools")
            for name in names:
                for sku in skus:
                    try_folder = progam_files_folder + "\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\" %(name, sku)
                    try_vcvarsall = "VCVARSALL.BAT"
                    if os.path.exists(try_folder + try_vcvarsall):
                        return try_folder
        return ""
            
                        
    def findProgramFilesFolder(self):
        env = os.environ
        if "64bit" == platform.architecture()[0]:
            if "ProgramFiles(x86)" in env:
                progam_files_folder = env["ProgramFiles(x86)"]
            else:
                progam_files_folder = "C:\Program Files (x86)"
        else:
            if "ProgramFiles" in env:
                progam_files_folder = env["ProgramFiles"]
            else:
                progam_files_folder = "C:\Program Files"
        
        return progam_files_folder
    
    def retrieveCMakeVersion(self):
        cmake_ver = subprocess.check_output([self.cmake_path, "--version"]).decode()
        if len(cmake_ver) == 0:
            LogError("Could NOT find CMake. Please install CMake 3.9+, set its path into  CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
        cmake_ver = cmake_ver.split()[2]
        cmake_ver_components = cmake_ver.split('.')
        return int(cmake_ver_components[0]+cmake_ver_components[1])
    
    def findCMake(self):
        cmake_loc = subprocess.check_output(self.where_cmd + " cmake", shell = True).decode()
        if len(cmake_loc) == 0:
            LogError("Could NOT find CMake. Please install CMake 3.9+, set its path into  CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
        return cmake_loc.split(self.sep)[0]
        
    def retrieveClangVersion(self, path = ""):
        if("android" == self.target_platform):
            android_ndk_path = os.environ["ANDROID_NDK"]
            prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
            prebuilt_clang_path = prebuilt_llvm_path+"\\prebuilt\\windows\\bin"
            if not os.path.isdir(prebuilt_clang_path):
                prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows-x86_64\\bin"
            clang_path = prebuilt_clang_path + "\\clang"
        else:
            clang_path = path + "clang"
        clang_ver = subprocess.ckeck_output([clang_path, "--version"]).decode()
        clang_ver_tokens = clang_ver.split()
        for i in range(0, len(clang_ver_tokens)):
            if "version" == clang_ver_tokens[i]:
                clang_ver_components = clang_ver_tokens[i + 1].split(".")
                break;
        return int(clang_ver_components[0] + clang_ver_components[1])
        
    def retrieveGCCVersion(self):
        gcc_ver = subprocess.check_output([self.findGCC(), "-dumpfullversion"]).decode()
        gcc_ver_components = gcc_ver.split(".")
        return int(gcc_ver_components[0] + gcc_ver_components[1])
        
    def getBuildDir(self, arch, config = None):
        env = os.environ
        if "BUILD_DIR" in env:
            build_dir_name = env["BUILD_DIR"]
        else:
            build_dir_name = "%s_%s%d_%s_%s" % (self.project_type, self.compiler_name, self.compiler_version, self.target_platform, arch)
            if not(config is None):
                build_dir_name += "-"+config
        return build_dir_name
    
    def MSBUILDAddBuildCommand(self, batch_cmd, sln_name, proj_name, config, arch=""):
        batch_cmd.addCommand('@SET VisualStudioVersion=%d.0' % self.vs_version)
        if len(proj_name) != 0:
            file_name = "%s.%s" % (proj_name, self.proj_ext_name)
        else:
            file_name = "%s.sln" % sln_name
        config_str = "Configuration=%s" % config
        if len(arch) != 0:
            config_str = "%s,Platform=%s" % (config_str, arch)
        
        batch_cmd.addCommand('@MSBuild %s /nologo /m:%d /v:m /p:%s' %(file_name, self.jobs, config_str))
        batch_cmd.addCommand('@if ERRORLEVEL 1 exit /B 1')
        
class BatchCommand:
    def __init__(self, host_platform):
        self.commands_ = []
        self.host_platform = host_platform
    
    def addCommand(self, cmd):
        self.commands_ += [cmd]
        
    def execute(self):
        batch_file = "ari_build."
        if "win" == self.host_platform:
            batch_file += "bat"
        else:
            batch_file += "sh"
        batch_f = open(batch_file, "w")
        print([cmd_line + "\n" for cmd_line in self.commands_])
        batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
        batch_f.close()
        if "win" == self.host_platform:
            ret_code = subprocess.call(batch_file, shell = True)
        else:
            subprocess.call("chmod 777 "+batch_file, shell = True)
            ret_code = subprocess.call("./" + batch_file, shell = True)
        os.remove(batch_file)
        return ret_code
    
        
def buildAProject(name, build_path, build_info, compiler_info, additional_options = ""):
    curdir = os.path.abspath(os.curdir)
    toolset_name = ""
    if 0 == build_info.project_type.find("vs"):
        toolset_name = "-T v%s" % build_info.compiler_version
        toolset_name += ",host=x64"
    elif ("android" == build_info.target_platform):
        android_ndk_path = os.environ["ANDROID_NDK"]
        prebuilt_llvm_path = android_ndk_path+"\\toolchains\\llvm"
        toolset_name = "clang"
    
    if(build_info.compiler_name != "vc") or (build_info.project_type == "ninja"):
        additional_options += " -AIR_ARCH_NAME:STRING=\"%s\"" % compiler_info.arch
    if "android" == build_info.target_platform:
        print("")
    
    if build_info.compiler_name == "vc":
        if "x64" == compiler_info.arch:
            vc_option = "amd64"
            vc_arch = "x64"
        elif "arm" == compiler_info.arch:
            vc_option = "amd64_arm"
            vc_arch = "AMR"
        elif "arm64" == compiler_info.arch:
            vc_option = "amd64_arm64"
            vc_arch = "ARM64"
        else:
            LogError("Unsupported VS architecture.\n")
        if len(compiler_info.vcvarsall_options) > 0:
            vc_option += " %s" % compiler_info.vcvarsall_options
    if build_info.multi_config:
        if 0 == build_info.project_type.find("vs"):
            additional_options += " -A %s" % vc_arch
        
        build_dir = "%s/Build/%s" % (build_path, build_info.getBuildDir(compiler_info.arch))
        if build_info.is_clean:
            print("Cleaning %s..." % name)
            sys.stdout.flush()
            
            if os.path.isdir(build_dir):
                shutil.rmtree(build_dir)
        else:
            print("Building %s..." % name)
            sys.stdout.flush()
            
            if not os.path.exists(build_dir):
                os.makedirs(build_dir)
            
            build_dir = os.path.abspath(build_dir)
            os.chdir(build_dir)
            
            cmake_cmd = BatchCommand(build_info.host_platform)
            new_path = sys.exec_prefix
            if len(compiler_info.compiler_root) > 0:
                new_path += ";" + compiler_info.compiler_root
            if "win" == build_info.host_platform:
                
                cmake_cmd.addCommand('@SET PATH=%s;%%PATH%%' % new_path)
                if 0 == build_info.project_type.find("vs"):
                    cmake_cmd.addCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, compiler_info.vcvarsall_options))
                    cmake_cmd.addCommand('@CD /d "%s"' % build_dir)
            cmake_cmd.addCommand('"%s" -G "%s" %s %s ../CMake' % (build_info.cmake_path, compiler_info.generator, toolset_name, additional_options))
            print("11111111111111111")
            if cmake_cmd.execute() != 0:
                LogWarning("Config %s failed, retry 1...\n" % name)
                if cmake_cmd.execute() != 0:
                    LogWarning("Config %s failed, retry 2...\n" % name)
                    if cmake_cmd.execute() != 0:
                        LogError("Config %s failed.\n" % name)
            # build_cmd = BatchCommand(build_info.host_platform)
            # if 0 == build_info.project_type.find("vs"):
                # build_cmd.addCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
                # build_cmd.addCommand('@CD /d "%s"' % build_dir)
            # for config in build_info.cfg:
                # if 0 == build_info.project_type.find("vs"):
                    # build_info.MSBUILDAddBuildCommand(build_cmd, name, "ALL_BUILD", config, vc_arch)
                # elif "xcode" == build_info.project_type:
                    # build_info.XCodeBuildAddBuildCommand(build_cmd, "ALL_BUILD", config)
            # if build_cmd.execute() != 0:
                # LogError("Build %s failed.\n" % name)
            
            # os.chdir(curdir)
            
            print("")
            sys.stdout.flush()
    else:
        if build_info.project_type == "ninja":
            make_name = "ninja"
        else:
            if "win" == build_info.host_platform:
                if build_info.target_platform == "android":
                    prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows"
                    if not os.path.isdir(prebuilt_make_path):
                        prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows-x86_64"
                    make_name = prebuilt_make_path + "\\bin\\make.exe"
                else:
                    make_name = "mingw32-make.exe"
            else:
                make_name = "make"
        for config in build_info.cfg:
            build_dir = "%s/Build/%s" %(build_path, build_info.getBuildDir(compiler_info.arch, config))
            if build_info.is_clean:
                print("Cleaning %s %s..." %(name, config))
                sys.stdout.flush()
                
                if os.path.isdir(build_dir):
                    shutil.rmtree(build_dir)
            else:
                print("Building %s %s..." %(name, config))
                sys.stdout.flush()
                
                if not os.path.exists(build_dir):
                    os.makedirs(build_dir)
                    if("clang" == build_info.compiler_name) and(build_info.target_platform != "android"):
                        env = os.environ
                        if not ("CC" in env):
                            additional_options += " -DCMAKE_C_COMPILER=clang"
                        if not ("CXX" in env):
                            additional_options += " -DCMAKE_CXX_COMPILER=clang++"
    
                build_dir = os.path.abspath(build_dir)
                os.chdir(build_dir)
                additional_options += " -DCMAKE_BUILD_TYPE:STRING=\"s\"" % config
                if "android" == build_info.target_platform:
                    if "x86" == compiler_info.arch:
                        abi_arch = "x86"
                        toolchain_arch = "x86"
                    elif "x86_64" == compiler_info.arch:
                        abi_arch = "x86_64"
                        toolchain_arch = "x86_64"
                    elif "arm64-v8a" == compiler_info.arch:
                        abi_arch = "arm64-v8a"
                        toolchain_arch = "aarch64-linux-android"
                    else:
                        LogError("Unsupported Android architecture.\n")
                    additional_options += " -DANDROID_STL=c++_static -DANDROID_ABI=\"%s\" -DANDROID_TOOLCHAIN_NAME=%s-clang" %(abi_arch, toolchain_arch)
                
                cmake_cmd = BatchCommand(build_info.host_platform)
                new_path = sys.exec_prefix
                if len(compiler_info.compiler_root) > 0:
                    new_path += ";" + compiler_info.compiler_root
                if build_info.compiler_name == "vc":
                    cmake_cmd.addCommand('@SET PATH=%s;%%PATH%%' % new_path)
                    cmake_cmd.addCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
                    cmake_cmd.addCommand('@CD /d "%s"' % build_dir)
                    additional_options += " -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe"
                cmake_cmd.addCommand('"%s" -G "%s" %s ../CMake' % (build_info.cmake_path, compiler_info.generator, additional_options))
                
                if cmake_cmd.execute() != 0:
                    LogWarning("Config %s failed, retry 1...\n" %name)
                    if cmake_cmd.execute() != 0:
                        LogWarning("Config %s failed, retry 2...\n" %name)
                        if cmake_cmd.execute() != 0:
                            LogError("Config %s failed.\n" % name)
                            
                # build_cmd = BatchCommand(build_info.host_platform)
                # if build_info.compiler_name == "vc":
                    # build_cmd.addCommand('@CALL "%s%s" %s' %(compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
                    # build_cmd.addCommand('@CD /d "%s"' % build_dir)
                # build_info.makeAddBuildCommand(build_cmd, make_name)
                # if build_cmd.execute() != 0:
                    # LogError("Build %s failed.\n" % name)
                
                # os.chdir(curdir)
                
                print("")
                sys.stdout.flush()
                
            
if __name__ == "__main__":
    build_info = BuildInfo.fromArgv(sys.argv)
    additional_options  = ""
    for compiler_info in build_info.compilers:
        buildAProject("Air", ".", build_info, compiler_info, additional_options)
        
            