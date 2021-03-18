#include "compiler/compiler.hh"

#include <string>
#include <fstream>
#include <iostream>

#include "tsupport.hh"

std::string create_code(std::string const& code, int n = 1)
{
    std::string filename = "/tmp/sample" + std::to_string(n) + ".z80";
    std::ofstream f(filename);
    f.write(code.data(), code.size());
    return filename;
}

int main()
{
    // simple nop file
    std::cout << "Simple 'nop' source file (sanity check): ";
    CompilerResult r = compile({ { create_code(" nop"), 0x0 } });
    ASSERT_Q(std::vector<uint8_t>{ 0 }, r.binary);
    ASSERT_Q((Files { { "sample1.z80", 1 } }), r.debug.files);
    ASSERT_Q((SourceAddress  { " nop", 0, { 0 } }), r.debug.source.at("sample1.z80").at(1));
    ASSERT_Q((SourceLine { "sample1.z80", 1 }), r.debug.location.at(0));
    std::cout << '\n';
    
    // file with labels
    std::cout << "File with labels: ";
    r = compile({ { create_code(R"(
    nop
main:
    nop
    jp main
    )"), 0x0 } });
    ASSERT_Q((std::vector<uint8_t>{ 0x0, 0x0, 0xc3, 0x1, 0x0 }), r.binary);
    ASSERT_Q((Files { { "sample1.z80", 6 } }), r.debug.files);
    ASSERT_Q((SourceAddress  { "main:", {}, {} }), r.debug.source.at("sample1.z80").at(3));
    ASSERT_Q((SourceAddress  { "    jp main", 2, { 0xc3, 0x1, 0x0 } }), r.debug.source.at("sample1.z80").at(5));
    ASSERT_Q((SourceLine { "sample1.z80", 5 }), r.debug.location.at(2));
    std::cout << '\n';
    
    // compilation error
    std::cout << "Compilation error: ";
    r = compile({ { create_code(" invalid"), 0x0 } });
    ASSERT_Q(true, r.error.has_value());
    ASSERT_Q(true, !r.error.value().empty());
    std::cout << '\n';
    
    // test import files
    std::cout << "Import files: ";
    create_code(R"(
dest:
    ret
    )", 2);
    r = compile({ { create_code(R"(
    jp dest
    include sample2.z80
    )"), 0x0 } });
    if (r.error.has_value()) {
        std::cerr << r.error.value() << "\n";
        exit(EXIT_FAILURE);
    }
    ASSERT_Q((std::vector<uint8_t>{ 0xc3, 0x3, 0x0, 0xc9 }), r.binary);
    ASSERT_Q((Files { { "sample1.z80", 4 }, { "sample2.z80", 4 } }), r.debug.files);
    ASSERT_Q((SourceAddress  { "dest:", {}, {} }), r.debug.source.at("sample2.z80").at(2));
    ASSERT_Q((SourceAddress  { "    jp dest", 0, { 0xc3, 0x3, 0x0 } }), r.debug.source.at("sample1.z80").at(2));
    ASSERT_Q((SourceLine { "sample2.z80", 3 }), r.debug.location.at(3));
    
    // TODO - test multiple files
}

