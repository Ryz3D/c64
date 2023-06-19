import os

for f in os.listdir("bin"):
    with open("bin/" + f, "rb") as fl:
        binary = fl.read()
    name = f.split(".")[0]
    strdata = ", ".join([str(x) for x in binary])
    with open("src/" + name + "_gen.c", "w") as fl:
        fl.write(f"const uint8_t {name}[] = {{\n{strdata}\n}};\n")
