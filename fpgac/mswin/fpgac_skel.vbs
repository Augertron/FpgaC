'VBScript fpga_skel.vbs
Option Explicit

Dim FPGAC_DIR, FPGAC
'either use the makefile to update FPGAC_DIR or do it by hand. It is the directory of the FPGAC executable.
'FPGAC_DIR="LIB_defined_in_the_makefile/"
FPGAC_DIR=".\"
'FPGAC= FPGAC_DIR & "fpgac.compiler"
' Under Windows executables have a .exe extension
FPGAC=FPGAC_DIR & "fpgac.exe"
Const ForAppending = 8

Dim objFSO, WshShell, objArgs

Set objFSO = CreateObject("Scripting.FileSystemObject")
Set WshShell = WScript.CreateObject("WScript.Shell")

Dim temp
temp=WshShell.ExpandEnvironmentStrings("%tmp%")
if temp = "" then
    temp="."
end if

If not objFSO.FileExists(FPGAC) Then
    Wscript.Echo FPGAC + "not found"
    Wscript.Quit
End If

'Wscript.echo "debug FPGAC=" & FPGAC

Dim toss, parttype, optimize, format, nocpp, nocpp_found, cppargs, cpp, args

toss=1
parttype="4003pc84"
optimize=false
format="xnf"
nocpp=0
nocpp_found=0
'cppargs="-DFPGAC=FPGACv1.0 -DPORT_REGISTERED=0x%x -DPORT_PIN=0x%x -DPORT_REGISTERED_AND_PIN=0x%x -DPORT_WIRE=0x0 -DPORT_PULLUP=0x%x -DPORT_PULLDOWN=0x%x PORT_REGISTERED, PORT_PIN, (PORT_REGISTERED | PORT_PIN), PORT_PULLUP, PORT_PULLDOWN"
'***either the cppargs is to long or the syntax at the end is causing problems
cppargs="-DFPGAC=FPGACv1.0 -DPORT_REGISTERED=0x%x -DPORT_PIN=0x%x -DPORT_REGISTERED_AND_PIN=0x%x -DPORT_WIRE=0x0 -DPORT_PULLUP=0x%x -DPORT_PULLDOWN=0x%x"

If objFSO.FileExists("\lib\cpp") Then
    cpp="\lib\cpp"
Elseif objFSO.FileExists("\usr\lib\cpp") Then
    cpp="\usr\lib\cpp"
Elseif objFSO.FileExists("\usr\bin\cpp") Then
    cpp="\usr\bin\cpp"
Else
    ' hope its found on path
    cpp="cpp"
End If
'Wscript.echo "debug cpp=" & cpp & " lenargs=" & len(cppargs)

Set objArgs = WScript.Arguments ' create object with collection
Dim i, realname
i = 0

If objArgs.Count <> 0 Then
  Dim opt
  While i <> objArgs.Count - 1
      'WScript.Echo "debug arg is " & objArgs(i)
      opt=Mid(objArgs(i),1,1)
      If (opt = "-") Then
        select case Mid(objArgs(i),1,2)
  	  case "-v"	toss=""

	  case "-O"	optimize=force
			args=args & objArgs(i) ' no space after option

	  case "-S"	xnfonly=1

	  case "-a"	nocpp=1

	  case "-b"	realname=objArgs(i)

          case "-D", "-U", "-I"
			cppargs=cppargs + " " & objArgs(i) ' no space after option

          case "-c", "-d", "-f", "-s", "-m", "-r", "-F", "-T"
			args=args & objArgs(i) ' no space after option

	  case "-t"
                        if ObjArgs(i) = "-target" then
                            args=args + objArgs(i) & " " & objArgs(i+1)
			    format=objArgs(i+1)
                            'wscript.echo "format is " & format
			    i=i+1
                        else
                            WshShell.echo "target option should be -target not " & objargs(i)
         	            WshShell.run "cmd /k " & FPGAC & " -usage"
	 		    Wscript.Quit
                        end if

	  case "-p"	parttype=objArgs(i+1)
			i=i+1

          case else
                WshShell.echo "unknown arg " & objargs(i)
                WshShell.run "cmd /k " & FPGAC & " -usage"
		Wscript.Quit
        end select
        i=i+1
      else
	break
      end if
  Wend
     ' what remains should be the filename(s)
  If i < (objArgs.Count - 1) Then
        args=args & "-c"
  End If
Else
  Wscript.Echo "Missing filename"
  WshShell.run "cmd /k " & FPGAC & " -usage"
  Wscript.Quit
End If

Dim tempout
tempout=temp & "\fpgac$$.out"

Private sub execCmd(cmdStr, Saveto, append)
    Dim rc, tempfile
'Begin
    'Wscript.echo "debug exec=" & CmdStr & " to " & Saveto
    if Saveto = "" then
        tempfile=temp & "\$$TEMP$$.txt"
    else
        tempfile=Saveto
    end if
    if append = true then
        rc=WshShell.run ("cmd /c " & cmdStr & ">>" & tempfile,0,true)
    else
        rc=WshShell.run ("cmd /c " & cmdStr & ">" & tempfile,0,true)
    end if
    If rc <> 0 Then
        WScript.Echo "Failed [" & rc & "] executing " & cmdStr
        WScript.Quit
    End if
    if (tempfile <> saveto) then
	'Wscript.echo "deleteing " & tempfile & " not " & saveto
        objFSO.DeleteFile(tempfile)
    end if
End Sub

Private function execCmdretVal(cmdStr)
    Dim rc, tempfile, result, objTextFile
'Begin
    'Wscript.echo "debug exec=" & CmdStr
    tempfile=temp & "\$$TEMP$$.txt"
    rc=WshShell.run ("cmd /c " & cmdStr & ">" & tempfile,0,true)
    If rc <> 0 Then
        WScript.Echo "Failed [" & rc & "] executing " & cmdStr
        WScript.Quit
    End if
    Set objTextFile = objFSO.OpenTextFile(tempfile, 1)
    result=objTextFile.ReadLine
    objTextFile.Close
    'Wscript.echo "debug result=" & Result
    objFSO.DeleteFile(tempfile)
    execCmdretVal=result
End Function

Private Sub runFPGAC(inputfile)
    Dim tempfile
'Begin
    'Wscript.echo "debug runFPGAC=" & inputfile
    If nocpp = 0 then
        If nocpp_found = 0 then
            tempfile=temp& "\fpgacin$$.c"
'           Dim tempfile2
'           tempfile2=temp& "\fpgacsed$$.c"
'           call execCmd("sed ""s/^#pragma/$pragma/"" " & inputfile, tempfile, false)
'           call execCmd(cpp & " " & cppargs & " " & tempfile, tempfile2, false) 
'           call execCmd("sed ""s/^$pragma/#pragma/"" " & tempfile2, tempfile, false)
            ' either 3 commands or one large one
            call execCmd("sed ""s/^#pragma/$pragma/"" " & inputfile & "|" &_
                         cpp & " " & cppargs &_
                         "| sed ""s/^$pragma/#pragma/"" ", tempfile, false)
        Else
            Wscript.Echo "No cpp found"
            tempfile=inputfile
        end if
    Else
        tempfile=inputfile
    end if
    call execCmd(FPGAC & " -a -b" & inputfile & " -p " & parttype & " " & args & " " & tempfile, tempout, false)
    if (tempfile <> inputfile) then
	'Wscript.echo "deleteing " & tempfile & " not " & inputfile
        objFSO.DeleteFile(tempfile)
    End if
End Sub

Dim j, first_inputfile, base_outputfile
first_inputfile=ObjArgs(i)
base_outputfile=execCmdretVal("basename " & first_inputfile & " .c")
'WScript.echo "debug base=" & base_outputfile & " firstin=" & first_inputfile

if Mid(format,len(format)-2,3) = "vqm" then
           ' better be stratix_vqm or fpgac will complain
        'Wscript.Echo "debug format vqm"
        call runFPGAC(first_inputfile)
        call execCmd("sed -e ""/Start of debug output/,$d"" " & tempout, base_outputfile &".vqm", false)
        objFSO.DeleteFile(tempout)
	
elseif Mid(format,1,3) = "cnf" then
        'this branch of the script is not tested Feburary 2006
        'Wscript.Echo "debug format cnf"
        objFSO.DeleteFile(base_outputfile & ".cnf")
        For j=i to ObjArgs.Count - 1
            If Mid(ObjArgs(j),len(ObjArgs(j))-3, 4) = ".cnf" then
                if objFSO.FileExists(ObjArgs(j)) then
                    call execCmd("type " & objargs(j), base_outputfile & ".cnf", true)
		else
		    Wscript.Echo "fpgac: " & ObjArgs(j) & ": can't open file"
                    objFSO.DeleteFile(base_outputfile &".cnf")
		    Wscript.Quit
		end if
            ElseIf Mid(ObjArgs(j),len(ObjArgs(j))-1, 2) = ".c" then
                call runFPGAC(ObjArgs(j))
                call execCmd ("type " & tempout, base_outputfile & ".cnf", true)
                objFSO.DeleteFile(tempout)
            end if
	Next
	
elseif Mid(format,1,3) = "xnf" then
        'Wscript.Echo "debug format xnf"
        call execCmd("echo LCANET, 4", base_outputfile & ".xnf", false)
        'the use of multiple input files in this branch of the script is not tested Feburary 2006
        For j=i to ObjArgs.Count - 1
            If Mid(ObjArgs(j),len(ObjArgs(j))-3, 4) = ".xnf" then
                if objFSO.FileExists(ObjArgs(j)) then
                    call execCmd ("sed -e ""/EOF/,$d"" -e ""/^LCANET/d"" -e ""/^PART/d"" -e ""/^PWR/d"" " & ObjArgs(j), base_outputfile & ".xnf",true)
  		else
		    Wscript.Echo "fpgac: " & ObjArgs(j) & ": can't open file"
                    objFSO.DeleteFile(base_outputfile & ".xnf")
		    Wscript.Quit
		end if

            ElseIf Mid(ObjArgs(j),len(ObjArgs(j))-1, 2) = ".c" then
                runFPGAC(ObjArgs(j))
                call execCmd("sed -e ""/EOF/,$d"" -e ""/^LCANET/d"" " & tempout, base_outputfile & ".xnf",true)
                objFSO.DeleteFile(tempout)
	    End if
	Next
        call execCmd("echo EOF", base_outputfile & ".xnf", true)
	
elseif Mid(format,1,3) = "vhd" or Mid(format,1,4) = "vhdl" then
        'Wscript.Echo "debug format vhd[l]"
        call runFPGAC(first_inputfile)
        call execCmd("sed -e ""/Start of debug output/,$d"" " & tempout, base_outputfile & ".vhd", false)
        objFSO.DeleteFile(tempout)

else
            ' fpgac will probably complain
        'Wscript.Echo "debug unknown format " & format
        call runFPGAC(first_inputfile)
	
End if
' exit 0

