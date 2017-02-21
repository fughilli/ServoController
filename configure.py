#!/usr/bin/python

from ninja_syntax import Writer
import os, sys

MCU = "msp430g2231"

source_dirs = [
        ".",
#        "util"
]

exclude_files = [
        ]

include_dirs = source_dirs + [
            "/usr/msp430/include"
        ]

defines = [
        "__" + MCU.upper() + "__",
        ]

def subst_ext(fname, ext):
    return os.path.splitext(fname)[0] + ext

def get_sources():
    fnames = []
    for d in source_dirs:
        for f in os.listdir(d):
            fnames.append(os.path.join(d, f))

    for excfile in exclude_files:
        fnames = filter(lambda x : not x.endswith(excfile), fnames)
    return fnames

def get_includes():
    return " ".join(map(lambda x : "-I"+x, include_dirs))

def get_defines():
    return " ".join(map(lambda x : "-D"+x, defines))

cflags = ("-g -c -O3 -ffunction-sections -fdata-sections " +
          "-fsingle-precision-constant -std=c99 -DF_CPU=16000000L " +
          "-mmcu=" + MCU + " " +
          get_defines() + " " + get_includes())

cxxflags = ("-g -c -O3 -std=c++0x -fno-rtti -fno-exceptions " +
            "-ffunction-sections -fdata-sections " +
            "-fsingle-precision-constant " +
            "-DF_CPU=16000000L -mmcu=" + MCU + " " +
            get_defines() + " " + get_includes())

lflags = ("-g -O3 -Wl,--gc-sections,-u,main " +
          "-fsingle-precision-constant -mmcu=" + MCU + " " +
          "-L$lib_path -L$ldscript_dev_path -T$ldscript_path/msp430.x ")

#/lib/libcrt0.a
#/lib/mmpy-16/libgcc.a
#/lib/mmpy-16/libgcov.a
#/lib/mmpy-16/libcrt0.a
#/lib/mmpy-16/libcrt0dwdt.a

def write_buildfile():
    with open("build.ninja", "w") as buildfile:
        n = Writer(buildfile)

        # Variable declarations
        n.variable("lib_path", "/usr/msp430/lib") #"/usr/lib/gcc/msp430/4.6.3")
        n.variable("ldscript_dev_path", "/usr/msp430/lib/ldscripts/" + MCU) 
        n.variable("ldscript_path", "/usr/msp430/lib/ldscripts") 
        n.variable("tc_path", "/usr/bin")
        n.variable("cflags", cflags)
        n.variable("cxxflags", cxxflags)
        n.variable("lflags", lflags)

        # Rule declarations
        n.rule("cxx",
               command = "msp430-g++ $cxxflags -c $in -o $out")

        n.rule("cc",
               command = "msp430-gcc $cflags -c $in -o $out")

        n.rule("cl",
               command = "msp430-gcc $lflags $in -o $out -lm -lgcc -lc")

        n.rule("oc",
               command = "msp430-objcopy -O binary $in $out")

        n.rule("cdb",
              command = "ninja -t compdb cc cxx > compile_commands.json")

        n.rule("cscf",
              command = "find " + " ".join(set(source_dirs + include_dirs)) +
                        " -regex \".*\\(\\.c\\|\\.h\\|.cpp\\|.hpp\\)$$\" -and " +
                        "-not -type d > $out")

        n.rule("cscdb",
              command = "cscope -bq")

        # Build rules
        n.build("compile_commands.json", "cdb")
        n.build("cscope.files", "cscf")
        n.build(["cscope.in.out", "cscope.po.out", "cscope.out"], "cscdb",
                "cscope.files")

        objects = []

        def cc(name):
            ofile = subst_ext(name, ".o")
            n.build(ofile, "cc", name)
            objects.append(ofile)
        def cxx(name):
            ofile = subst_ext(name, ".o")
            n.build(ofile, "cxx", name)
            objects.append(ofile)
        def cl(oname, ofiles):
            n.build(oname, "cl", ofiles)

        sources = get_sources()
        map(cc, filter(lambda x : x.endswith(".c"), sources))
        map(cxx, filter(lambda x : x.endswith(".cpp"), sources))

        cl("main.elf", objects)

        n.build("main.bin", "oc", "main.elf")

if __name__ == "__main__":
    write_buildfile()

