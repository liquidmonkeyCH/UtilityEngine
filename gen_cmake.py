# -*- coding: UTF-8 -*-
# 根据vcxproj生成cmake文件，便于linux下程序编译

import os
import sys
import xml.etree.ElementTree as ET
import codecs
import re
import json

common = 'common'
proj_source_dir = "${PROJECT_SOURCE_DIR}/"

class cmake_tool(object):
    def __init__(self):
        self.compile_files = []
        self.project = ""
        self.compile_mode = None
        self.root = None  # xml parse
        self.proj_conf = {}
        self.common_conf = {}
        self.definitions = ""
        self.include_directories = ""
        self.link_directories = ""
        self.link_libraries = ""
        self.cxx_flags = ""
        self.library = None

    def writecmakefile(self, proj_path):
        cmake_str = ""
        cmake_str += 'cmake_minimum_required(VERSION 2.8)\n'
        cmake_str += 'project(%s)\n' % self.project
        if self.definitions != "":
            cmake_str += 'add_definitions(%s)\n' % self.definitions
        if self.include_directories != "":
            cmake_str += 'include_directories(%s)\n' % self.include_directories
        if self.link_directories != "":
            cmake_str += 'link_directories(%s)\n' % self.link_directories
        cmake_str += 'set(SOURCE_FILE %s)' % " ".join(self.compile_files)
        cmake_str += '\n'
        cmake_str += '''
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
        \n'''
        if self.cxx_flags != "":
            cmake_str += 'set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} %s")\n' % self.cxx_flags
   
        if not self.library or self.library == "":
            cmake_str += 'set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin_%s)\n' % self.compile_mode
            cmake_str += 'add_executable(%s ${SOURCE_FILE})\n' % self.project
            cmake_str += 'target_link_libraries(%s  %s)\n' % (
                self.project, self.link_libraries)
        else:
            cmake_str += 'set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin_%s/libs)\n' % self.compile_mode
            cmake_str += 'add_library(%s %s ${SOURCE_FILE})\n' % (
                self.project, self.library)

        output_path = proj_path + "build_" + self.compile_mode
        if not os.path.exists(output_path):
            os.mkdir(output_path)
        
        with open(proj_path + '/CMakeLists.txt', 'w') as f:
            f.write(cmake_str)
            f.flush()


    def read_common_conf(self):
        contents = ""
        with codecs.open("./common_linux_conf.json", "r", "utf-8") as f:
            contents = f.read()
            self.common_conf = json.loads(contents)


    def read_proj_conf(self, proj_path):
        contents = ""
        with codecs.open(proj_path + "proj_linux_conf.json", "r", "utf-8") as f:
            contents = f.read()
            self.proj_conf = json.loads(contents)

    def check_create_path(self,path,islinkpath):
        if not islinkpath or path[0] != ".":
            return path
        return proj_source_dir + path


    def get_conf_value(self,mode,key,islinkpath = False):
        contents = ""
        if self.common_conf[common].get(key) != None and self.common_conf[common][key] != "":
            contents += self.check_create_path(self.common_conf[common][key],islinkpath)
            
        if self.proj_conf[common].get(key) != None and self.proj_conf[common][key] != "":
            contents += " "
            contents += self.check_create_path(self.proj_conf[common][key],islinkpath)
            
        if self.common_conf.get(mode) != None:
            if self.common_conf[mode].get(key) != None and self.common_conf[mode][key] != "":
                contents += " "
                contents += self.check_create_path(self.common_conf[mode][key],islinkpath)
        else:
            print("[warring] common_linux_conf.json compile mode(%s) not found!" % mode)
                  
        if self.proj_conf.get(mode) != None:
            if self.proj_conf[mode].get(key) != None and self.proj_conf[mode][key] != "":
                contents += " "
                contents += self.check_create_path(self.proj_conf[mode][key],islinkpath)
        else:
            print("[warring] proj_linux_conf.json compile mode(%s) not found!" % mode)
                  
        return contents

    def prase_conf(self,compile_mode):
        if compile_mode == common:
            print("[error] compile mode can not be common!")
            return False
        if self.proj_conf.get(common) == None:
            print(self.proj_conf.get(common))
            print("[error] load proj_linux_conf.json error! key(common) not found!")
            return False
        if self.common_conf.get(common) == None:
            print("[error] load common_linux_conf.json error! key(common) not found!")
            return False
        self.definitions = self.get_conf_value(compile_mode,'defines')
        self.include_directories = self.get_conf_value(compile_mode,'include_path')
        self.link_directories = self.get_conf_value(compile_mode,'link_path',True)
        self.link_libraries = self.get_conf_value(compile_mode,'link_libs')
        self.cxx_flags = self.get_conf_value(compile_mode,'cxx_flags')
        self.library = self.proj_conf.get('library')
        return True

    def loadxml(self, xml_file, proj_path):
        if not os.path.exists(proj_path):
            print("path not exist")
            return
        self.project = sys.argv[1]
        self.compile_mode = sys.argv[2]

        contents = ""
        with codecs.open(xml_file, "r", "utf-8") as f:
            contents = f.read()
        pattern = re.compile(r'xmlns=".*"', re.I)
        xml_contents = re.sub(pattern, "", contents)
        self.root = ET.fromstring(xml_contents)
        compile_files_item = self.root.findall('ItemGroup/ClCompile')
        for item in compile_files_item:
            for k, v in item.items():
                self.compile_files.append(v.replace('\\', '/'))

        self.read_common_conf()
        self.read_proj_conf(proj_path)
        if not self.prase_conf(self.compile_mode):
            return
        
        self.writecmakefile(proj_path)


if __name__ == '__main__':
    tool = cmake_tool()
    proj_path = "./"+sys.argv[1]+"/"
    vcproj_file = proj_path+sys.argv[1]+".vcxproj"
    tool.loadxml(vcproj_file, proj_path)
