# import time
import os
import paramiko
import shutil

def main():
    # shutil.copy(os.path.expanduser("~/GOPATH/src/github.com/morentharia/simple_pe_loader/Debug/simplepeloader.exe"),
    #             os.path.expanduser("~/hack/vmware_share/main.exe"))
    # shutil.copy(os.path.expanduser("~/GOPATH/src/github.com/morentharia/simple_pe_loader/hello.exe"),
    #             os.path.expanduser("~/hack/vmware_share/hello.exe"))
    # shutil.copy(os.path.expanduser("~/GOPATH/src/github.com/morentharia/pepepepe/shellcode.bin"),
    #             os.path.expanduser("~/hack/vmware_share/shellcode.bin"))
    try:
        #vmware win10
        # host = '192.168.1.150'
        #virtualbox win10
        host = '192.168.1.93'
        user = 'mor'
        secret = '1'
        port = 22


        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=host, username=user, password=secret, port=port)

        commands_list = [
            # '''taskkill.exe /f /im main.exe ''',
            # '''taskkill.exe /f /im notepad.exe ''',

            #VirtualBox
            # '''Z:\\main.exe ''',
            # '''Copy-Item '\\\\vmware-host\\Shared Folders\\shared\\main.exe' -Destination C:\\ ''',

            '''$env:Path = 'C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin;' + $env:Path; '''
            ''' MSBuild; '''
            '''cd z:\\; '''
            '''pwd ;'''
            ''' MSBuild .\\simplepeloader\\simplepeloader.vcxproj -p:Configuration=Debug -p:Platform=x86;'''
            # '''C:\\PSTools\\PsExec64.exe -i 1 -s \'C:\\main.exe\' ''',

            # ''' C:\\\\Program Files\\\\Microsoft Visual Studio\\\\2022\\\\Community\\\\MSBuild\\\\Current\\\\Bin\\\\MSBuild ''',
            # '''MSBuild .\simplepeloader\simplepeloader.vcxproj -p:Configuration=Debug -p:Platform=x86'''

            # '''MSBuild .\simplepeloader\simplepeloader.vcxproj -p:Configuration=Debug -p:Platform=x86'''
            # '''Copy-Item 'Z:\\main.exe' -Destination C:\\ ''',
            # '''Copy-Item 'Z:\\hello.exe' -Destination C:\\ ''',
            # '''C:\\main.exe ''',
            # '''C:\\PSTools\\PsExec64.exe -i 1 -s \'C:\\main.exe\' ''',
            #VmWare
            # '''Copy-Item '\\\\vmware-host\\Shared Folders\\shared\\main.exe' -Destination C:\\ ''',
            # '''Copy-Item '\\\\vmware-host\\Shared Folders\\shared\\shellcode.bin' -Destination C:\\ ''',
            # # '''C:\\main.exe ''',
            # '''C:\\PsExec64.exe -i 1 -s \'C:\\main.exe\' ''',
        ]
        for ccc in commands_list:
            print(f"PS> {ccc}")
            stdin, stdout, stderr = client.exec_command(ccc, get_pty=True)
            err_str = stderr.read()
            data = (stdout.read() + err_str).decode(errors='replace')
            # if "Процесс не может получить доступ к файлу" in data:
            #     print(data)
            #     print("exit")
            #     break

            print(data)
    except KeyboardInterrupt:
        print("bye bye")
    finally:
        # client.send(chr(3))
        channel = client.invoke_shell()
        channel.send(b'0x3')
        print("close client")
        channel.transport.close()
        client.close()

if __name__ == '__main__':
    main()
