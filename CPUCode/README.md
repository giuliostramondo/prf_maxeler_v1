# PRF Testbench integrated with Maxeler
Testbench to check the correctness of a prf implementation on a maxeler board.

```
Usage: ../RunRules/DFE/binaries/PRF_Complete [Options]

 -N <num>	 Change the horizontal size of the input matrix (default 9)
 -M <num>	 Change the vertical size of the input matrix (default 9)
 -p <num>	 Change the vertical size of the PRF (default 3)
 -q <num>	 Change the horizontal size of the PRF (default 3)
 -s <num>	 Change the schema used by the PRF (default 0 -> RECTANGLE_ONLY)
        	  other schemes 1->RECT_ROW, 2->RECT_COL, 3->ROW_COL, 4->RECT_TRECT
```
