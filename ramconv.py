with open("freeram.prg", "rb") as fl:
    binary = fl.read()
strdata = ", ".join([str(x) for x in binary])
with open("src/ram.c", "w") as fl:
    fl.write(f"#include \"ram.h\"\n\nuint8_t ram[] = {{{strdata}}};\n")
