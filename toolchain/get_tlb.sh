(echo "info tlb"; sleep 1; echo "quit") | telnet 10.194.0.165 45454 2>&1 | tee tlb_out.txt


echo "TLB dump saved to tlb_out.txt"