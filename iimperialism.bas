10 REM            *** IIMPERIALISM ***
20 REM            TACIANO DRECKMANN PEREZ
30 REM            tdperez@hotmail.com
40 HOME : POKE  - 16368,0 : REM            (CLS ON SOME EMUS)
50 DIM MN$(12),ML(12)
60 FOR I = 1 TO 12 : READ MN$(I) : NEXT 
70 DATA            "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
80 FOR I = 1 TO 12 : READ ML(I) : NEXT 
90 DATA            31,28,31,30,31,30,31,31,30,31,30,31
100 REM            --- DATE ---
110 D = 15 : M = 2 : Y = 1864
120 REM            --- WAREHOUSE STOCKS ---
130 TI = 10 : LU = 0 : FU = 0 : CO = 10 : FB = 0 : CL = 0
140 REM            --- ORDERS (FOR NEXT TURN) ---
150 TT = 0 : CT = 0 : PL = 0 : PF = 0 : PG = 0 : PC = 0
160 REM            --- TRANSPORT CAPACITY ---
165 TCAP = 20
167 REM            --- FACTORY CAPACITIES ---
168 SCAP = 1 : FCAP = 1 : WCAP = 1 : CCAP = 1
169 REM            SAWMILL, FURNITURE, TEXTILE WORKSHOP, CLOTHES
170 REM            --- PROVINCE SUPPLY (CHANGES EACH TURN) ---
175 PT = 0 : PCOT = 0
180 REM            --- FIRST TURN:GENERATESUPPLY,PROCESS( OR DERSINIT0) -  -  - 
190 GOSUB 6000 : REM            GEN PROV SUPPLY
200 GOSUB 7000 : REM            PROCESS THIS TURN (ORDERS->RESULTS)
210 GOSUB 5700 : REM            CLEAR ORDERS AFTER PROCESS
220 REM            --- UI LOOP (PLACE ORDERS, THEN NEXT TURN) ---
230 GOSUB 2000 : REM            DRAW SCREEN
240 GOSUB 3000 : REM            PROMPT
250 GOTO 240
1990 REM            ================== UI: DRAW  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  = 
2000 HOME 
2010 GOSUB 9000 : REM            HEADER LINE WITH DATE
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
3020 IF A$ = "T" OR A$ = "t" THEN  GOSUB 4000 : GOSUB 2000 : RETURN 
3030 IF A$ = "P" OR A$ = "p" THEN  GOSUB 4700 : GOSUB 2000 : RETURN 
3040 IF A$ = "N" OR A$ = "n" THEN  GOSUB 8000 : GOSUB 6000 : GOSUB 7000 : GOSUB 5700 : GOSUB 2000 : RETURN 
3050 PRINT "Please type T, P, or N."
3060 RETURN 
3990 REM            ================== TRANSPORT ENTRY ==================
4000 GOSUB 4200 : REM            SHOW TRANSPORT ORDERS SCREEN
4010 INPUT A$
4020 IF  LEN (A$) > 0 THEN A$ =  LEFT$ (A$,1)
4030 IF A$ = "A" OR A$ = "a" THEN  GOSUB 4400 : GOTO 4000
4040 IF A$ = "T" OR A$ = "t" THEN  GOSUB 4520 : GOTO 4000
4045 IF A$ = "C" OR A$ = "c" THEN  RETURN 
4050 PRINT "Please type A, T, or C."
4060 GOTO 4010
4070 RETURN 
4390 REM            ================== ADD TRANSPORT CAPACITY ==================
4400 PRINT 
4410 INPUT "Add how much transport capacity? ";X
4420 IF X < 0 THEN X = 0
4430 TCAP = TCAP +  INT (X)
4440 RETURN 
4490 REM            ================== CHANGE TRANSPORT ORDERS ==================
4520 PRINT  : PRINT "Set TRANSPORT orders for next turn."
4530 PRINT "Note: Provinces produce new random amounts each turn."
4535 PRINT "Transport capacity: ";TCAP
4540 INPUT "Timber to transport (0..999)? ";X
4550 IF X < 0 THEN X = 0
4560 IF X > 999 THEN X = 999
4570 TEMPT =  INT (X)
4580 INPUT "Cotton to transport (0..999)? ";X
4590 IF X < 0 THEN X = 0
4600 IF X > 999 THEN X = 999
4610 TEMPC =  INT (X)
4615 REM            CHECK CAPACITY LIMIT
4620 IF TEMPT + TEMPC > TCAP THEN  PRINT "ERROR: Total transport orders (";TEMPT + TEMPC;") exceed capacity (";TCAP;")!" : GOTO 4540
4625 REM            ACCEPT VALID ORDERS
4630 TT = TEMPT : CT = TEMPC
4640 RETURN 
4190 REM            ================== TRANSPORT ORDERS SCREEN ==================
4200 HOME 
4210 GOSUB 9000 : REM            HEADER LINE WITH DATE
4220 PRINT " _____________________________________ "
4230 PRINT "|TRANSPORT ORDERS                     |"
4240 PRINT "| Timber ";TT;"                          |"
4250 PRINT "| Cotton ";CT;"                          |"
4260 PRINT " ------------------------------------- "
4270 PRINT "Transport capacity: ";TCAP
4280 PRINT 
4290 PRINT "Do you want to Add capacity, change"
4300 PRINT "Transport orders, or Cancel?"
4310 RETURN 
4690 REM            ================== PRODUCTION ENTRY ==================
4700 GOSUB 4900 : REM            SHOW PRODUCTION ORDERS SCREEN
4710 INPUT A$
4720 IF  LEN (A$) > 0 THEN A$ =  LEFT$ (A$,1)
4730 IF A$ = "C" OR A$ = "c" THEN  GOSUB 5300 : GOTO 4700
4740 IF A$ = "A" OR A$ = "a" THEN  GOSUB 5500 : GOTO 4700
4750 IF A$ = "R" OR A$ = "r" THEN  RETURN 
4760 PRINT "Please type C, A, or R."
4770 GOTO 4710
4780 RETURN 
4890 REM            ================== PRODUCTION ORDERS SCREEN ==================
4900 HOME 
4910 GOSUB 9000 : REM            HEADER LINE WITH DATE
4920 PRINT " _____________________________________ "
4930 PRINT "|PRODUCTION ORDERS                    |"
4940 PRINT " ------------------------------------- "
4950 PRINT "|Sawmill       Cap:";SCAP;"  Orders:";PL;"      |"
4960 PRINT "|  (2 Timber -> 1 Lumber)             |"
4970 PRINT " ------------------------------------- "
4980 PRINT "|Furniture Fac Cap:";FCAP;"  Orders:";PF;"      |"
4990 PRINT "|  (2 Lumber -> 1 Furniture)          |"
5000 PRINT " ------------------------------------- "
5010 PRINT "|Textile Works Cap:";WCAP;"  Orders:";PG;"      |"
5020 PRINT "|  (2 Cotton -> 1 Fabric)             |"
5030 PRINT " ------------------------------------- "
5040 PRINT "|Clothes Fact  Cap:";CCAP;"  Orders:";PC;"      |"
5050 PRINT "|  (2 Fabric -> 1 Clothes)            |"
5060 PRINT " ------------------------------------- "
5070 PRINT 
5080 PRINT "Do you want to Change orders, Add"
5090 PRINT "capacity, or Return?"
5100 RETURN 
5290 REM            ================== CHANGE PRODUCTION ORDERS ==================
5300 PRINT  : PRINT "Set PRODUCTION orders for each factory."
5310 INPUT "Sawmill orders (max ";SCAP;")? ";X
5320 IF X < 0 THEN X = 0
5330 IF X > SCAP THEN  PRINT "ERROR: Orders exceed capacity!" : GOTO 5310
5340 PL =  INT (X)
5350 INPUT "Furniture Factory orders (max ";FCAP;")? ";X
5360 IF X < 0 THEN X = 0
5370 IF X > FCAP THEN  PRINT "ERROR: Orders exceed capacity!" : GOTO 5350
5380 PF =  INT (X)
5390 INPUT "Textile Workshop orders (max ";WCAP;")? ";X
5400 IF X < 0 THEN X = 0
5410 IF X > WCAP THEN  PRINT "ERROR: Orders exceed capacity!" : GOTO 5390
5420 PG =  INT (X)
5430 INPUT "Clothes Factory orders (max ";CCAP;")? ";X
5440 IF X < 0 THEN X = 0
5450 IF X > CCAP THEN  PRINT "ERROR: Orders exceed capacity!" : GOTO 5430
5460 PC =  INT (X)
5470 RETURN 
5490 REM            ================== ADD PRODUCTION CAPACITY ==================
5500 PRINT  : PRINT "Add capacity to which factory?"
5510 PRINT "1=Sawmill, 2=Furniture, 3=Textile, 4=Clothes"
5520 INPUT "Factory (1-4)? ";X
5530 IF X < 1 OR X > 4 THEN  PRINT "Invalid factory!" : GOTO 5520
5540 INPUT "Add how much capacity? ";Y
5550 IF Y < 0 THEN Y = 0
5560 IF X = 1 THEN SCAP = SCAP +  INT (Y)
5570 IF X = 2 THEN FCAP = FCAP +  INT (Y)
5580 IF X = 3 THEN WCAP = WCAP +  INT (Y)
5590 IF X = 4 THEN CCAP = CCAP +  INT (Y)
5600 RETURN 
5690 REM            ================== CLEAR ORDERS ==================
5700 TT = TT * 0 : CT = CT * 0 : PL = PL * 0 : PF = PF * 0 : PG = PG * 0 : PC = PC * 0
5710 RETURN 
5990 REM            ================== GENERATE PROVINCE SUPPLY ==================
6000 REM            NEW SUPPLY EACH TURN (HIDDEN; ORDERS CAP AGAINST THIS)
6010 PT =  INT ( RND (1) * 16) + 5 : REM            5..20 TIMBER
6020 PCOT =  INT ( RND (1) * 16) + 5 : REM            5..20 COTTON
6030 RETURN 
6990 REM            ================== PROCESS TURN ==================
7000 REM           1) TRANSPORT ARRIVES (CAPPED BY PROV SUPPLY)
7010 BT = TT
7015 AC = CT
7020 IF BT > PT THEN BT = PT
7030 IF AC > PCOT THEN AC = PCOT
7040 TI = TI + BT : CO = CO + AC
7050 REM           2) PRODUCTION:SAWMILL (2 TIMBER -> 1 LUMBER)
7060 MAKE = PL : IF MAKE >  INT (TI / 2) THEN MAKE =  INT (TI / 2)
7070 TI = TI - MAKE * 2 : LU = LU + MAKE
7080 REM           3) PRODUCTION:LUMBER -  > FURNITURE(2:1)
7090 REM           MAX FURN:MIN(PF, INT (LU / 2))
7100 MAXF = PF : IF MAXF >  INT (LU / 2) THEN MAXF =  INT (LU / 2)
7110 LU = LU - MAXF * 2 : FU = FU + MAXF
7120 REM           4) PRODUCTION:TEXTILE WORKSHOP (2 COTTON -> 1 FABRIC)
7130 MAKE = PG : IF MAKE >  INT (CO / 2) THEN MAKE =  INT (CO / 2)
7140 CO = CO - MAKE * 2 : FB = FB + MAKE
7150 REM           5) PRODUCTION:FABRIC -  > CLOTHES(2:1)
7160 MAXC = PC : IF MAXC >  INT (FB / 2) THEN MAXC =  INT (FB / 2)
7170 FB = FB - MAXC * 2 : CL = CL + MAXC
7180 RETURN 
7990 REM            ================== ADVANCE DATE BY 7 DAYS ==================
8000 D = D + 7
8010 GOSUB 8500 : REM            MONTHLEN IN MLEN
8020 IF D <=  MLEN THEN 8070
8030 D = D - MLEN : M = M + 1
8040 IF M <=  12 THEN 8010
8050 M = 1 : Y = Y + 1 : GOTO 8010
8070 RETURN 
8490 REM            ================== MONTH LENGTH (WITH LEAP) ==================
8500 MLEN = ML(M)
8510 IF M <> 2 THEN 8520
8515 IF (Y -  INT (Y / 4) * 4) = 0 THEN MLEN = 29
8520 RETURN 
8990 REM            ================== HEADER ==================
9000 PRINT "President TACIANO of Haxaco  ";D;" ";MN$(M);" ";Y
9010 RETURN 