#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <winnt.h>

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("missing path argument\n");
        return 1;
    }
    FILE* exe_file = fopen(argv[1], "rb");
    /* FILE* exe_file = fopen("C:\\hello.exe", "rb"); */
    if (!exe_file) {
        printf("error opening file\n");
        return 1;
    }

    fseek(exe_file, 0L, SEEK_END);
    long int file_size = ftell(exe_file);
    fseek(exe_file, 0L, SEEK_SET);

    char* PE_data = (char *)malloc(file_size + 1);

    size_t n_read = fread(PE_data, 1, file_size, exe_file);
    if (n_read != file_size) {
        printf("reading error (%d)\n", n_read);
        return 1;
    }

    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)PE_data;
    IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(((char*)dos_header) + dos_header->e_lfanew);

    char* ImageBase = (char*)VirtualAlloc(NULL,
            nt_header->OptionalHeader.SizeOfImage,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE);
    if (ImageBase == NULL) {
        return NULL;
    }

    memcpy(ImageBase, PE_data, nt_header->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)(nt_header + 1);

    for (int i = 0; i < nt_header->FileHeader.NumberOfSections; ++i) {
        char* dest = ImageBase + sections[i].VirtualAddress;

        if (sections[i].SizeOfRawData > 0) {
            memcpy(dest, PE_data + sections[i].PointerToRawData, sections[i].SizeOfRawData);
        }
        else {
            memset(dest, 0, sections[i].Misc.VirtualSize);
        }
    }

    IMAGE_DATA_DIRECTORY* data_directory = nt_header->OptionalHeader.DataDirectory;

    IMAGE_IMPORT_DESCRIPTOR* import_descriptors = (IMAGE_IMPORT_DESCRIPTOR*)(ImageBase + data_directory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (int i = 0; import_descriptors[i].OriginalFirstThunk != 0; ++i) {

        char* module_name = ImageBase + import_descriptors[i].Name;
        HMODULE import_module = LoadLibraryA(module_name);
        if (import_module == NULL) {
            return NULL;
        }
        printf("module_name: %s\n", module_name);

        IMAGE_THUNK_DATA* lookup_table = (IMAGE_THUNK_DATA*)(ImageBase + import_descriptors[i].OriginalFirstThunk);
        IMAGE_THUNK_DATA* address_table = (IMAGE_THUNK_DATA*)(ImageBase + import_descriptors[i].FirstThunk);
        for (int i = 0; lookup_table[i].u1.AddressOfData != 0; ++i) {
            void* function_handle = NULL;
            DWORD lookup_addr = lookup_table[i].u1.AddressOfData;

            IMAGE_IMPORT_BY_NAME* image_import = (IMAGE_IMPORT_BY_NAME*)(ImageBase + lookup_addr);
            char* func_name = (char*)&(image_import->Name);
            printf("\t func_name: %s\n", func_name);
            function_handle = (void*)GetProcAddress(import_module, func_name);
            if (function_handle == NULL) {
                return NULL;
            }

            address_table[i].u1.Function = (DWORD)function_handle;
        }
    }

    /* __asm{ */
    /*     nop; */
    /*     nop; */
    /*     nop; */
    /*     nop; */
    /*     nop; */
    /*     nop; */
    /*     int 3; */
    /* } */

    DWORD delta_VA_reloc = ((DWORD)ImageBase) - nt_header->OptionalHeader.ImageBase;

    if (data_directory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress != 0 && delta_VA_reloc != 0) {
        IMAGE_BASE_RELOCATION* p_reloc = (IMAGE_BASE_RELOCATION*)(ImageBase + data_directory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

        while (p_reloc->VirtualAddress != 0) {
            DWORD size = (p_reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
            WORD* reloc = (WORD*)(p_reloc + 1);
            for (int i = 0; i < size; ++i) {
                int type = reloc[i] >> 12;
                int offset = reloc[i] & 0x0fff;
                DWORD* change_addr = (DWORD*)(ImageBase + p_reloc->VirtualAddress + offset);

                switch (type) {
                case IMAGE_REL_BASED_HIGHLOW:
                    *change_addr += delta_VA_reloc;
                    break;
                default:
                    break;
                }
            }

            p_reloc = (IMAGE_BASE_RELOCATION*)(((DWORD)p_reloc) + p_reloc->SizeOfBlock);
        }
    }

    ((void (*)(void)) (ImageBase + nt_header->OptionalHeader.AddressOfEntryPoint) )();
    return 0;
}
