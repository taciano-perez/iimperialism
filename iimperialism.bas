10 REM            *** IIMPERIALISM ***
20 REM            TACIANO DRECKMANN PEREZ
30 REM            tdperez@hotmail.com
40 HOME 
 : POKE  - 16368,0
 : REM            (CLS ON SOME EMUS)
50 DIM MN$(12),ML(12)
60 FOR I = 1 TO 12
 : READ MN$(I)
 : NEXT 
70 DATA            "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
80 FOR I = 1 TO 12
 : READ ML(I)
 : NEXT 
90 DATA            31,28,31,30,31,30,31,31,30,31,30,31
100 REM            --- DATE ---
110 D = 15
  : M = 2
  : Y = 1864
120 REM            --- WAREHOUSE STOCKS ---
130 TI = 0
  : LU = 0
  : FU = 0
  : CO = 0
  : FB = 0
  : CL = 0
140 REM            --- ORDERS (FOR NEXT TURN) ---
150 TT = 0
  : CT = 0
  : PL = 0
  : PF = 0
  : PG = 0
  : PC = 0
160 REM            --- PROVINCE SUPPLY (CHANGES EACH TURN) ---
170 PT = 0
  : PCOT = 0
180 REM            --- FIRST TURN:GENERATESUPPLY,PROCESS( OR DERSINIT0) -  -  - 
190 GOSUB 6000
  : REM            GEN PROV SUPPLY
200 GOSUB 7000
  : REM            PROCESS THIS TURN (ORDERS->RESULTS)
210 GOSUB 5000
  : REM            CLEAR ORDERS AFTER PROCESS
220 REM            --- UI LOOP (PLACE ORDERS, THEN NEXT TURN) ---
230 GOSUB 2000
  : REM            DRAW SCREEN
240 GOSUB 3000
  : REM            PROMPT
250 GOTO 240
1990 REM            ================== UI: DRAW  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  = 
2000 HOME 
2010 GOSUB 9000
   : REM            HEADER LINE WITH DATE
2020 PRINT " _____________________________________ "
2030 PRINT "|Warehouse                            |"
2040 PRINT "| Timber ";TI;"  Lumber ";LU;" Furniture ";FU;"      |"
2050 PRINT "| Cotton ";CO;"  Fabric ";FB;" Clothes      ";CL;"   |"
2060 PRINT " ------------------------------------- "
2070 PRINT " _____________________________________ "
2080 PRINT "|Transport Orders | Production Orders |"
2090 PRINT "| Timber ";TT;"   | Lumber ";PL;" Furniture ";PF;"   |"
2100 PRINT "| Cotton ";CT;"   | Fabric ";PG;" Clothes  ";PC;"    |"
2110 PRINT " ------------------------------------- "
2120 PRINT "Do you have Transport,Production orders"
2130 PRINT "or Next turn?"
2140 RETURN 
2990 REM            ================== UI:PROMPT =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  = 
3000 INPUT A$
3010 IF  LEN (A$) > 0 THEN A$ =  LEFT$ (A$,1)
3020 IF A$ = "T" OR A$ = "t" THEN  GOSUB 4000
   : GOSUB 2000
   : RETURN 
3030 IF A$ = "P" OR A$ = "p" THEN  GOSUB 4500
   : GOSUB 2000
   : RETURN 
3040 IF A$ = "N" OR A$ = "n" THEN  GOSUB 8000
   : GOSUB 6000
   : GOSUB 7000
   : GOSUB 5000
   : GOSUB 2000
   : RETURN 
3050 PRINT "Please type T, P, or N."
3060 RETURN 
3990 REM            ================== TRANSPORT ENTRY ==================
4000 PRINT 
   : PRINT "Set TRANSPORT orders for next turn."
4010 PRINT "Note: Provinces produce new random amounts each turn."
4020 INPUT "Timber to transport (0..999)? ";X
4030 IF X < 0 THEN X = 0
4040 IF X > 999 THEN X = 999
4050 TT =  INT (X)
4060 INPUT "Cotton to transport (0..999)? ";X
4070 IF X < 0 THEN X = 0
4080 IF X > 999 THEN X = 999
4090 CT =  INT (X)
4100 RETURN 
4490 REM            ================== PRODUCTION ENTRY ==================
4500 PRINT 
   : PRINT "Set PRODUCTION orders for next turn."
4510 PRINT "Costs: 1 Timber->1 Lumber, 2 Lumber->1 Furniture"
4520 PRINT "       1 Cotton->1 Fabric, 2 Fabric->1 Clothes"
4530 INPUT "Produce Lumber (uses Timber)? ";X
   : IF X < 0 THEN X = 0
4540 PL =  INT (X)
4550 INPUT "Produce Furniture (uses Lumber x2)? ";X
   : IF X < 0 THEN X = 0
4560 PF =  INT (X)
4570 INPUT "Produce Fabric (uses Cotton)? ";X
   : IF X < 0 THEN X = 0
4580 PG =  INT (X)
4590 INPUT "Produce Clothes (uses Fabric x2)? ";X
   : IF X < 0 THEN X = 0
4600 PC =  INT (X)
4610 RETURN 
4990 REM            ================== CLEAR ORDERS ==================
5000 TT = TT * 0
   : CT = CT * 0
   : PL = PL * 0
   : PF = PF * 0
   : PG = PG * 0
   : PC = PC * 0
5010 RETURN 
5990 REM            ================== GENERATE PROVINCE SUPPLY ==================
6000 REM            NEW SUPPLY EACH TURN (HIDDEN; ORDERS CAP AGAINST THIS)
6010 PT =  INT ( RND (1) * 16) + 5
   : REM            5..20 TIMBER
6020 PCOT =  INT ( RND (1) * 16) + 5
   : REM            5..20 COTTON
6030 RETURN 
6990 REM            ================== PROCESS TURN ==================
7000 REM           1) TRANSPORT ARRIVES (CAPPED BY PROV SUPPLY)
7010 BT = TT
7015 AC = CT
7020 IF BT > PT THEN BT = PT
7030 IF AC > PCOT THEN AC = PCOT
7040 TI = TI + BT
   : CO = CO + AC
7050 REM           2) PRODUCTION:TIMBER -  > LUMBER
7060 MAKE = PL
   : IF MAKE > TI THEN MAKE = TI
7070 TI = TI - MAKE
   : LU = LU + MAKE
7080 REM           3) PRODUCTION:LUMBER -  > FURNITURE(2:1)
7090 REM           MAX FURN:MIN(PF, INT (LU / 2))
7100 MAXF = PF
   : IF MAXF >  INT (LU / 2) THEN MAXF =  INT (LU / 2)
7110 LU = LU - MAXF * 2
   : FU = FU + MAXF
7120 REM           4) PRODUCTION:COTTON -  > FABRIC
7130 MAKE = PG
   : IF MAKE > CO THEN MAKE = CO
7140 CO = CO - MAKE
   : FB = FB + MAKE
7150 REM           5) PRODUCTION:FABRIC -  > CLOTHES(2:1)
7160 MAXC = PC
   : IF MAXC >  INT (FB / 2) THEN MAXC =  INT (FB / 2)
7170 FB = FB - MAXC * 2
   : CL = CL + MAXC
7080 REM            3) PRODUCTION:LUMBER -  > FURNITURE(2:1)
7090 REM            MAX FURN:MIN(PF, INT (LU / 2))
7100 MAXF = PF
   : IF MAXF >  INT (LU / 2) THEN MAXF =  INT (LU / 2)
7110 LU = LU - MAXF * 2
   : FU = FU + MAXF
7120 REM            4) PRODUCTION:COTTON -  > FABRIC
7130 MAKE = PG
   : IF MAKE > CO THEN MAKE = CO
7140 CO = CO - MAKE
   : FB = FB + MAKE
7150 REM            5) PRODUCTION:FABRIC -  > CLOTHES(2:1)
7160 MAXC = PC
   : IF MAXC >  INT (FB / 2) THEN MAXC =  INT (FB / 2)
7170 FB = FB - MAXC * 2
   : CL = CL + MAXC
7180 RETURN 
7990 REM            ================== ADVANCE DATE BY 7 DAYS ==================
8000 D = D + 7
8010 GOSUB 8500
   : REM            MONTHLEN IN MLEN
8020 IF D <  = MLEN THEN 8070
8030 D = D - MLEN
   : M = M + 1
8040 IF M <  = 12 THEN 8010
8050 M = 1
   : Y = Y + 1
   : GOTO 8010
8070 RETURN 
8490 REM            ================== MONTH LENGTH (WITH LEAP) ==================
8500 MLEN = ML(M)
8510 IF M = 2 THEN  IF (Y - ((Y\4) * 4)) = 0 THEN MLEN = 29
8520 RETURN 
8990 REM            ================== HEADER ==================
9000 PRINT "President TACIANO of Haxaco  ";D;" ";MN$(M);" ";Y
9010 RETURN 
