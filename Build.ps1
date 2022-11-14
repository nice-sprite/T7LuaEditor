
cmake --build W:\Priscilla\Build --target Priscilla -j 6
write-host $lastExitCode
# write-host $args[0]

if (( $args[0] -eq "run" ) -and ($lastExitCode -eq 0)) 
{
    start-process W:\Priscilla\Build\Priscilla.exe
}
