; Disassembly of the file "A:\rom.bin"
; 
; CPU Type: Z80
; 
; Created with dZ80 1.50
; 
; on Tuesday, 14 of August 2001 at 01:42 AM
; 
0000 210080    LD      HL,&8000
0003 110180    LD      DE,&8001
0006 1838      JR      &0040            ; (+&38)
0008 C3C20F    JP      &0FC2
000B 00        NOP     
000C 00        NOP     
000D 00        NOP     
000E 00        NOP     
000F 00        NOP     
0010 C31A0F    JP      &0F1A
0013 00        NOP     
0014 00        NOP     
0015 00        NOP     
0016 00        NOP     
0017 00        NOP     
0018 E3        EX      (SP),HL
0019 CD3F0F    CALL    &0F3F
001C 28FB      JR      Z,&0019          ; (-&05)
001E E3        EX      (SP),HL
001F C9        RET     

0020 F5        PUSH    AF
0021 79        LD      A,C
0022 D309      OUT     (&09),A
0024 78        LD      A,B
0025 C3980C    JP      &0C98
0028 CD8D0C    CALL    &0C8D
002B DB08      IN      A,(&08)
002D C9        RET     

002E 00        NOP     
002F 00        NOP     
0030 E7        RST     &20
0031 D308      OUT     (&08),A
0033 C9        RET     

0034 010204    LD      BC,&0402
0037 08        EX      AF,AF'
0038 00        NOP     
0039 00        NOP     
003A C30402    JP      &0204
003D 62        LD      H,D
003E 00        NOP     
003F 00        NOP     
0040 01FF7F    LD      BC,&7FFF
0043 71        LD      (HL),C
0044 EDB0      LDIR    
0046 210080    LD      HL,&8000
0049 44        LD      B,H
004A EDB0      LDIR    
004C 2100FB    LD      HL,&FB00
004F 1101FB    LD      DE,&FB01
0052 34        INC     (HL)
0053 01FF04    LD      BC,&04FF
0056 EDB0      LDIR    
0058 21F00F    LD      HL,&0FF0
005B 1100FB    LD      DE,&FB00
005E 0E46      LD      C,&46
0060 EDB0      LDIR    
0062 213610    LD      HL,&1036
0065 110000    LD      DE,&0000
0068 0E0B      LD      C,&0B
006A EDB0      LDIR    
006C 1109FC    LD      DE,&FC09
006F 0E84      LD      C,&84
0071 EDB0      LDIR    
0073 218CFB    LD      HL,&FB8C
0076 0606      LD      B,&06
0078 3630      LD      (HL),&30
007A 23        INC     HL
007B 10FB      DJNZ    &0078            ; (-&05)
007D 31FFFC    LD      SP,&FCFF
0080 3EFB      LD      A,&FB
0082 ED47      LD      I,A
0084 3260FB    LD      (&FB60),A
0087 ED7365FB  LD      (&FB65),SP
008B 3E01      LD      A,&01
008D 32B0FB    LD      (&FBB0),A
0090 ED5E      IM      2
0092 AF        XOR     A
0093 D328      OUT     (&28),A
0095 CF        RST     &08
0096 BC        CP      H
0097 3ECE      LD      A,&CE
0099 D311      OUT     (&11),A
009B 3E27      LD      A,&27
009D D311      OUT     (&11),A
009F 210800    LD      HL,&0008
00A2 54        LD      D,H
00A3 5C        LD      E,H
00A4 CF        RST     &08
00A5 81        ADD     A,C
00A6 3E10      LD      A,&10
00A8 D331      OUT     (&31),A
00AA 3E12      LD      A,&12
00AC D333      OUT     (&33),A
00AE 3E0F      LD      A,&0F
00B0 D331      OUT     (&31),A
00B2 3E87      LD      A,&87
00B4 D331      OUT     (&31),A
00B6 3E08      LD      A,&08
00B8 0680      LD      B,&80
00BA 21E80F    LD      HL,&0FE8
00BD 4E        LD      C,(HL)
00BE CD8D0C    CALL    &0C8D
00C1 23        INC     HL
00C2 04        INC     B
00C3 3D        DEC     A
00C4 20F7      JR      NZ,&00BD         ; (-&09)
00C6 01803B    LD      BC,&3B80
00C9 E7        RST     &20
00CA 018000    LD      BC,&0080
00CD 59        LD      E,C
00CE CDA60C    CALL    &0CA6
00D1 CF        RST     &08
00D2 BE        CP      (HL)
00D3 CF        RST     &08
00D4 D3CF      OUT     (&CF),A
00D6 BF        CP      A
00D7 FB        EI      
00D8 3A0040    LD      A,(&4000)
00DB B7        OR      A
00DC CC0140    CALL    Z,&4001
00DF 3E01      LD      A,&01
00E1 D323      OUT     (&23),A
00E3 CF        RST     &08
00E4 AF        XOR     A
00E5 CA5002    JP      Z,&0250
00E8 CF        RST     &08
00E9 BD        CP      L
00EA DF        RST     &18
00EB 0D        DEC     C
00EC 0A        LD      A,(BC)
00ED 2020      JR      NZ,&010F         ; (+&20)
00EF 2020      JR      NZ,&0111         ; (+&20)
00F1 2020      JR      NZ,&0113         ; (+&20)
00F3 2049      JR      NZ,&013E         ; (+&49)
00F5 6E        LD      L,(HL)
00F6 73        LD      (HL),E
00F7 65        LD      H,L
00F8 72        LD      (HL),D
00F9 74        LD      (HL),H
00FA 2064      JR      NZ,&0160         ; (+&64)
00FC 69        LD      L,C
00FD 73        LD      (HL),E
00FE 63        LD      H,E
00FF 2069      JR      NZ,&016A         ; (+&69)
0101 6E        LD      L,(HL)
0102 2064      JR      NZ,&0168         ; (+&64)
0104 72        LD      (HL),D
0105 69        LD      L,C
0106 76        HALT    
0107 65        LD      H,L
0108 2030      JR      NZ,&013A         ; (+&30)
010A 2061      JR      NZ,&016D         ; (+&61)
010C 6E        LD      L,(HL)
010D 64        LD      H,H
010E 0D        DEC     C
010F 0A        LD      A,(BC)
0110 2020      JR      NZ,&0132         ; (+&20)
0112 2020      JR      NZ,&0134         ; (+&20)
0114 2020      JR      NZ,&0136         ; (+&20)
0116 2020      JR      NZ,&0138         ; (+&20)
0118 70        LD      (HL),B
0119 72        LD      (HL),D
011A 65        LD      H,L
011B 73        LD      (HL),E
011C 73        LD      (HL),E
011D 2043      JR      NZ,&0162         ; (+&43)
011F 74        LD      (HL),H
0120 72        LD      (HL),D
0121 6C        LD      L,H
0122 2D        DEC     L
0123 42        LD      B,D
0124 52        LD      D,D
0125 45        LD      B,L
0126 41        LD      B,C
0127 4B        LD      C,E
0128 2074      JR      NZ,&019E         ; (+&74)
012A 6F        LD      L,A
012B 206C      JR      NZ,&0199         ; (+&6c)
012D 6F        LD      L,A
012E 61        LD      H,C
012F 64        LD      H,H
0130 0D        DEC     C
0131 0A        LD      A,(BC)
0132 0A        LD      A,(BC)
0133 54        LD      D,H
0134 41        LD      B,C
0135 54        LD      D,H
0136 55        LD      D,L
0137 4E        LD      C,(HL)
0138 47        LD      B,A
0139 2F        CPL     
013A 58        LD      E,B
013B 74        LD      (HL),H
013C 61        LD      H,C
013D 6C        LD      L,H
013E 204D      JR      NZ,&018D         ; (+&4d)
0140 4F        LD      C,A
0141 53        LD      D,E
0142 2031      JR      NZ,&0175         ; (+&31)
0144 2E32      LD      L,&32
0146 2020      JR      NZ,&0168         ; (+&20)
0148 2020      JR      NZ,&016A         ; (+&20)
014A 2020      JR      NZ,&016C         ; (+&20)
014C 2020      JR      NZ,&016E         ; (+&20)
014E 2843      JR      Z,&0193          ; (+&43)
0150 29        ADD     HL,HL
0151 2031      JR      NZ,&0184         ; (+&31)
0153 39        ADD     HL,SP
0154 3833      JR      C,&0189          ; (+&33)
0156 2031      JR      NZ,&0189         ; (+&31)
0158 39        ADD     HL,SP
0159 38B4      JR      C,&010F          ; (-&4c)
015B 180A      JR      &0167            ; (+&0a)
015D DF        RST     &18
015E 0D        DEC     C
015F 0A        LD      A,(BC)
0160 4D        LD      C,L
0161 4F        LD      C,A
0162 53        LD      D,E
0163 2031      JR      NZ,&0196         ; (+&31)
0165 2EB2      LD      L,&B2
0167 DF        RST     &18
0168 0D        DEC     C
0169 0A        LD      A,(BC)
016A 52        LD      D,D
016B 65        LD      H,L
016C 61        LD      H,C
016D 64        LD      H,H
016E 79        LD      A,C
016F 0D        DEC     C
0170 8A        ADC     A,D
0171 31FFFC    LD      SP,&FCFF
0174 FB        EI      
0175 CF        RST     &08
0176 A7        AND     A
0177 3A40FB    LD      A,(&FB40)
017A D7        RST     &10
017B 11B8FB    LD      DE,&FBB8
017E CF        RST     &08
017F 9D        SBC     A,L
0180 1A        LD      A,(DE)
0181 B7        OR      A
0182 28F3      JR      Z,&0177          ; (-&0d)
0184 CDDC01    CALL    &01DC
0187 13        INC     DE
0188 E65F      AND     &5F
018A D641      SUB     &41
018C 3867      JR      C,&01F5          ; (+&67)
018E 218202    LD      HL,&0282
0191 87        ADD     A,A
0192 4F        LD      C,A
0193 0600      LD      B,&00
0195 09        ADD     HL,BC
0196 46        LD      B,(HL)
0197 23        INC     HL
0198 66        LD      H,(HL)
0199 68        LD      L,B
019A 013404    LD      BC,&0434
019D ED42      SBC     HL,BC
019F 09        ADD     HL,BC
01A0 2005      JR      NZ,&01A7         ; (+&05)
01A2 01F201    LD      BC,&01F2
01A5 C5        PUSH    BC
01A6 E9        JP      (HL)
01A7 E5        PUSH    HL
01A8 D9        EXX     
01A9 210000    LD      HL,&0000
01AC 54        LD      D,H
01AD 5D        LD      E,L
01AE 44        LD      B,H
01AF 4D        LD      C,L
01B0 D9        EXX     
01B1 3E03      LD      A,&03
01B3 3249FB    LD      (&FB49),A
01B6 CF        RST     &08
01B7 B0        OR      B
01B8 281A      JR      Z,&01D4          ; (+&1a)
01BA CF        RST     &08
01BB AC        XOR     H
01BC E5        PUSH    HL
01BD D9        EXX     
01BE 3A49FB    LD      A,(&FB49)
01C1 FE03      CP      &03
01C3 2001      JR      NZ,&01C6         ; (+&01)
01C5 E1        POP     HL
01C6 FE02      CP      &02
01C8 2001      JR      NZ,&01CB         ; (+&01)
01CA D1        POP     DE
01CB FE01      CP      &01
01CD 2001      JR      NZ,&01D0         ; (+&01)
01CF C1        POP     BC
01D0 D9        EXX     
01D1 3D        DEC     A
01D2 20DF      JR      NZ,&01B3         ; (-&21)
01D4 B7        OR      A
01D5 21F201    LD      HL,&01F2
01D8 E3        EX      (SP),HL
01D9 E5        PUSH    HL
01DA D9        EXX     
01DB C9        RET     

01DC F5        PUSH    AF
01DD 2145FB    LD      HL,&FB45
01E0 CB56      BIT     2,(HL)
01E2 280C      JR      Z,&01F0          ; (+&0c)
01E4 0650      LD      B,&50
01E6 21B8FB    LD      HL,&FBB8
01E9 3E7F      LD      A,&7F
01EB A6        AND     (HL)
01EC 77        LD      (HL),A
01ED 23        INC     HL
01EE 10F9      DJNZ    &01E9            ; (-&07)
01F0 F1        POP     AF
01F1 C9        RET     

01F2 D27101    JP      NC,&0171
01F5 DF        RST     &18
01F6 2045      JR      NZ,&023D         ; (+&45)
01F8 72        LD      (HL),D
01F9 72        LD      (HL),D
01FA 6F        LD      L,A
01FB F2DF21    JP      P,&21DF
01FE 07        RLCA    
01FF 0D        DEC     C
0200 8A        ADC     A,D
0201 C37101    JP      &0171
0204 ED7365FB  LD      (&FB65),SP
0208 F3        DI      
0209 3177FB    LD      SP,&FB77
020C E5        PUSH    HL
020D D5        PUSH    DE
020E C5        PUSH    BC
020F F5        PUSH    AF
0210 2A65FB    LD      HL,(&FB65)
0213 CF        RST     &08
0214 B1        OR      C
0215 5F        LD      E,A
0216 23        INC     HL
0217 CF        RST     &08
0218 B1        OR      C
0219 57        LD      D,A
021A 23        INC     HL
021B 2265FB    LD      (&FB65),HL
021E 1B        DEC     DE
021F ED5377FB  LD      (&FB77),DE
0223 D9        EXX     
0224 08        EX      AF,AF'
0225 E5        PUSH    HL
0226 D5        PUSH    DE
0227 C5        PUSH    BC
0228 F5        PUSH    AF
0229 3165FB    LD      SP,&FB65
022C FDE5      PUSH    IY
022E DDE5      PUSH    IX
0230 2A5FFB    LD      HL,(&FB5F)
0233 ED57      LD      A,I
0235 F5        PUSH    AF
0236 7D        LD      A,L
0237 325FFB    LD      (&FB5F),A
023A 31FFFC    LD      SP,&FCFF
023D 2A5DFB    LD      HL,(&FB5D)
0240 7C        LD      A,H
0241 B5        OR      L
0242 3A5CFB    LD      A,(&FB5C)
0245 2801      JR      Z,&0248          ; (+&01)
0247 77        LD      (HL),A
0248 210000    LD      HL,&0000
024B CF        RST     &08
024C 99        SBC     A,C
024D C37101    JP      &0171
0250 31FFFC    LD      SP,&FCFF
0253 AF        XOR     A
0254 67        LD      H,A
0255 6F        LD      L,A
0256 3250FB    LD      (&FB50),A
0259 2251FB    LD      (&FB51),HL
025C 2100FE    LD      HL,&FE00
025F 2253FB    LD      (&FB53),HL
0262 E5        PUSH    HL
0263 CF        RST     &08
0264 A2        AND     D
0265 E1        POP     HL
0266 5E        LD      E,(HL)
0267 23        INC     HL
0268 56        LD      D,(HL)
0269 23        INC     HL
026A D5        PUSH    DE
026B 5E        LD      E,(HL)
026C 23        INC     HL
026D 56        LD      D,(HL)
026E 23        INC     HL
026F 4E        LD      C,(HL)
0270 23        INC     HL
0271 46        LD      B,(HL)
0272 23        INC     HL
0273 7E        LD      A,(HL)
0274 32B1FB    LD      (&FBB1),A
0277 E1        POP     HL
0278 C5        PUSH    BC
0279 AF        XOR     A
027A 47        LD      B,A
027B 4F        LD      C,A
027C CD8C04    CALL    &048C
027F C317FB    JP      &FB17
0282 2C        INC     L
0283 03        INC     BC
0284 49        LD      C,C
0285 03        INC     BC
0286 9D        SBC     A,L
0287 03        INC     BC
0288 B9        CP      C
0289 03        INC     BC
028A 02        LD      (BC),A
028B 04        INC     B
028C EC03FE    CALL    PE,&FE03
028F 03        INC     BC
0290 34        INC     (HL)
0291 04        INC     B
0292 F5        PUSH    AF
0293 01F501    LD      BC,&01F5
0296 F5        PUSH    AF
0297 01F501    LD      BC,&01F5
029A 5C        LD      E,H
029B 04        INC     B
029C F5        PUSH    AF
029D 01F501    LD      BC,&01F5
02A0 F5        PUSH    AF
02A1 01F501    LD      BC,&01F5
02A4 89        ADC     A,C
02A5 04        INC     B
02A6 F5        PUSH    AF
02A7 014D05    LD      BC,&054D
02AA F5        PUSH    AF
02AB 01F501    LD      BC,&01F5
02AE 91        SUB     C
02AF 04        INC     B
02B0 87        ADD     A,A
02B1 05        DEC     B
02B2 8C        ADC     A,H
02B3 05        DEC     B
02B4 93        SUB     E
02B5 05        DEC     B
02B6 5D        LD      E,L
02B7 01D706    LD      BC,&06D7
02BA 5E        LD      E,(HL)
02BB 0614      LD      B,&14
02BD 061A      LD      B,&1A
02BF 0F        RRCA    
02C0 5C        LD      E,H
02C1 0F        RRCA    
02C2 48        LD      C,B
02C3 0F        RRCA    
02C4 52        LD      D,D
02C5 0F        RRCA    
02C6 EB        EX      DE,HL
02C7 1C        INC     E
02C8 EE1C      XOR     &1C
02CA E8        RET     PE

02CB 04        INC     B
02CC F8        RET     M

02CD 04        INC     B
02CE 14        INC     D
02CF 0F        RRCA    
02D0 0F        RRCA    
02D1 0F        RRCA    
02D2 180F      JR      &02E3            ; (+&0f)
02D4 9D        SBC     A,L
02D5 0EA1      LD      C,&A1
02D7 0EA5      LD      C,&A5
02D9 0EE4      LD      C,&E4
02DB 0ED1      LD      C,&D1
02DD 0E1B      LD      C,&1B
02DF 1EDC      LD      E,&DC
02E1 1D        DEC     E
02E2 FD0E14    LD      C,&14
02E5 FB        EI      
02E6 1A        LD      A,(DE)
02E7 FB        EI      
02E8 21FB6A    LD      HL,&6AFB
02EB 0F        RRCA    
02EC 6F        LD      L,A
02ED 07        RLCA    
02EE 8D        ADC     A,L
02EF 1EA2      LD      E,&A2
02F1 1EA7      LD      E,&A7
02F3 1EAC      LD      E,&AC
02F5 1EB1      LD      E,&B1
02F7 1ECD      LD      E,&CD
02F9 1EAE      LD      E,&AE
02FB 0F        RRCA    
02FC 72        LD      (HL),D
02FD 1EC7      LD      E,&C7
02FF 0D        DEC     C
0300 7B        LD      A,E
0301 0E92      LD      C,&92
0303 0F        RRCA    
0304 2000      JR      NZ,&0306         ; (+&00)
0306 E1        POP     HL
0307 0B        DEC     BC
0308 3000      JR      NC,&030A         ; (+&00)
030A 93        SUB     E
030B 186D      JR      &037A            ; (+&6d)
030D 180A      JR      &0319            ; (+&0a)
030F 19        ADD     HL,DE
0310 04        INC     B
0311 19        ADD     HL,DE
0312 19        ADD     HL,DE
0313 19        ADD     HL,DE
0314 1A        LD      A,(DE)
0315 1B        DEC     DE
0316 F618      OR      &18
0318 BA        CP      D
0319 1845      JR      &0360            ; (+&45)
031B 1A        LD      A,(DE)
031C B6        OR      (HL)
031D 19        ADD     HL,DE
031E 60        LD      H,B
031F 0E26      LD      C,&26
0321 0F        RRCA    
0322 CE0A      ADC     A,&0A
0324 45        LD      B,L
0325 0C        INC     C
0326 04        INC     B
0327 40        LD      B,B
0328 0B        DEC     BC
0329 0E41      LD      C,&41
032B 0D        DEC     C
032C EB        EX      DE,HL
032D E5        PUSH    HL
032E 19        ADD     HL,DE
032F CF        RST     &08
0330 A9        XOR     C
0331 E1        POP     HL
0332 B7        OR      A
0333 ED52      SBC     HL,DE
0335 CF        RST     &08
0336 A9        XOR     C
0337 2B        DEC     HL
0338 2B        DEC     HL
0339 7C        LD      A,H
033A CB05      RLC     L
033C CE00      ADC     A,&00
033E 2005      JR      NZ,&0345         ; (+&05)
0340 7D        LD      A,L
0341 0F        RRCA    
0342 C3A50E    JP      &0EA5
0345 DF        RST     &18
0346 2D        DEC     L
0347 AD        XOR     L
0348 C9        RET     

0349 7D        LD      A,L
034A F5        PUSH    AF
034B E60F      AND     &0F
034D 6F        LD      L,A
034E 67        LD      H,A
034F F1        POP     AF
0350 E6F0      AND     &F0
0352 2805      JR      Z,&0359          ; (+&05)
0354 0F        RRCA    
0355 0F        RRCA    
0356 0F        RRCA    
0357 0F        RRCA    
0358 67        LD      H,A
0359 E5        PUSH    HL
035A 7D        LD      A,L
035B 0E28      LD      C,&28
035D CD7703    CALL    &0377
0360 E1        POP     HL
0361 D8        RET     C

0362 7C        LD      A,H
0363 0E29      LD      C,&29
0365 CD7703    CALL    &0377
0368 D8        RET     C

0369 7A        LD      A,D
036A B3        OR      E
036B C8        RET     Z

036C 3E40      LD      A,&40
036E D311      OUT     (&11),A
0370 7A        LD      A,D
0371 D311      OUT     (&11),A
0373 7B        LD      A,E
0374 D311      OUT     (&11),A
0376 C9        RET     

0377 FE09      CP      &09
0379 3F        CCF     
037A D8        RET     C

037B 218B03    LD      HL,&038B
037E 87        ADD     A,A
037F 85        ADD     A,L
0380 6F        LD      L,A
0381 3002      JR      NC,&0385         ; (+&02)
0383 B7        OR      A
0384 24        INC     H
0385 46        LD      B,(HL)
0386 23        INC     HL
0387 7E        LD      A,(HL)
0388 C3BC0F    JP      &0FBC
038B D0        RET     NC

038C 1D        DEC     E
038D 8E        ADC     A,(HL)
038E 1D        DEC     E
038F 68        LD      L,B
0390 1D        DEC     E
0391 34        INC     (HL)
0392 1D        DEC     E
0393 1A        LD      A,(DE)
0394 1D        DEC     E
0395 68        LD      L,B
0396 5D        LD      E,L
0397 34        INC     (HL)
0398 5D        LD      E,L
0399 1A        LD      A,(DE)
039A 5D        LD      E,L
039B 0D        DEC     C
039C 5D        LD      E,L
039D EB        EX      DE,HL
039E B7        OR      A
039F ED52      SBC     HL,DE
03A1 D8        RET     C

03A2 C5        PUSH    BC
03A3 E3        EX      (SP),HL
03A4 C1        POP     BC
03A5 EB        EX      DE,HL
03A6 03        INC     BC
03A7 B7        OR      A
03A8 ED52      SBC     HL,DE
03AA 19        ADD     HL,DE
03AB 3803      JR      C,&03B0          ; (+&03)
03AD CF        RST     &08
03AE B2        OR      D
03AF C9        RET     

03B0 EB        EX      DE,HL
03B1 09        ADD     HL,BC
03B2 EB        EX      DE,HL
03B3 09        ADD     HL,BC
03B4 2B        DEC     HL
03B5 1B        DEC     DE
03B6 CF        RST     &08
03B7 B3        OR      E
03B8 C9        RET     

03B9 1E00      LD      E,&00
03BB 011027    LD      BC,&2710
03BE CDD603    CALL    &03D6
03C1 01E803    LD      BC,&03E8
03C4 CDD603    CALL    &03D6
03C7 016400    LD      BC,&0064
03CA CDD603    CALL    &03D6
03CD 010A00    LD      BC,&000A
03D0 CDD603    CALL    &03D6
03D3 7D        LD      A,L
03D4 1810      JR      &03E6            ; (+&10)
03D6 3EFF      LD      A,&FF
03D8 B7        OR      A
03D9 ED42      SBC     HL,BC
03DB 3C        INC     A
03DC 30FA      JR      NC,&03D8         ; (-&06)
03DE 09        ADD     HL,BC
03DF 1C        INC     E
03E0 1D        DEC     E
03E1 2003      JR      NZ,&03E6         ; (+&03)
03E3 3C        INC     A
03E4 3D        DEC     A
03E5 C8        RET     Z

03E6 C630      ADD     A,&30
03E8 1E01      LD      E,&01
03EA D7        RST     &10
03EB C9        RET     

03EC EB        EX      DE,HL
03ED B7        OR      A
03EE ED52      SBC     HL,DE
03F0 D8        RET     C

03F1 79        LD      A,C
03F2 2002      JR      NZ,&03F6         ; (+&02)
03F4 12        LD      (DE),A
03F5 C9        RET     

03F6 44        LD      B,H
03F7 4D        LD      C,L
03F8 62        LD      H,D
03F9 6B        LD      L,E
03FA 13        INC     DE
03FB 77        LD      (HL),A
03FC 18AF      JR      &03AD            ; (-&51)
03FE 2277FB    LD      (&FB77),HL
0401 EB        EX      DE,HL
0402 F3        DI      
0403 7C        LD      A,H
0404 B5        OR      L
0405 280A      JR      Z,&0411          ; (+&0a)
0407 CF        RST     &08
0408 B1        OR      C
0409 225DFB    LD      (&FB5D),HL
040C 325CFB    LD      (&FB5C),A
040F 36FF      LD      (HL),&FF
0411 315FFB    LD      SP,&FB5F
0414 F1        POP     AF
0415 ED47      LD      I,A
0417 DDE1      POP     IX
0419 FDE1      POP     IY
041B F1        POP     AF
041C F1        POP     AF
041D C1        POP     BC
041E D1        POP     DE
041F E1        POP     HL
0420 D9        EXX     
0421 08        EX      AF,AF'
0422 F1        POP     AF
0423 C1        POP     BC
0424 D1        POP     DE
0425 ED7B65FB  LD      SP,(&FB65)
0429 2A77FB    LD      HL,(&FB77)
042C E5        PUSH    HL
042D 2A75FB    LD      HL,(&FB75)
0430 FB        EI      
0431 C317FB    JP      &FB17
0434 CF        RST     &08
0435 B0        OR      B
0436 210000    LD      HL,&0000
0439 1A        LD      A,(DE)
043A B7        OR      A
043B 281C      JR      Z,&0459          ; (+&1c)
043D 13        INC     DE
043E D630      SUB     &30
0440 FE0A      CP      &0A
0442 3F        CCF     
0443 D8        RET     C

0444 019A19    LD      BC,&199A
0447 ED42      SBC     HL,BC
0449 09        ADD     HL,BC
044A 3F        CCF     
044B D8        RET     C

044C 29        ADD     HL,HL
044D 44        LD      B,H
044E 4D        LD      C,L
044F 29        ADD     HL,HL
0450 29        ADD     HL,HL
0451 09        ADD     HL,BC
0452 4F        LD      C,A
0453 0600      LD      B,&00
0455 09        ADD     HL,BC
0456 30E1      JR      NC,&0439         ; (-&1f)
0458 C9        RET     

0459 CF        RST     &08
045A A9        XOR     C
045B C9        RET     

045C CF        RST     &08
045D A9        XOR     C
045E CF        RST     &08
045F B1        OR      C
0460 CF        RST     &08
0461 AB        XOR     E
0462 DF        RST     &18
0463 08        EX      AF,AF'
0464 88        ADC     A,B
0465 11B8FB    LD      DE,&FBB8
0468 CF        RST     &08
0469 9D        SBC     A,L
046A E5        PUSH    HL
046B CDDC01    CALL    &01DC
046E CF        RST     &08
046F AC        XOR     H
0470 3810      JR      C,&0482          ; (+&10)
0472 C1        POP     BC
0473 CF        RST     &08
0474 B0        OR      B
0475 28E5      JR      Z,&045C          ; (-&1b)
0477 FE2E      CP      &2E
0479 C8        RET     Z

047A CF        RST     &08
047B AD        XOR     L
047C 3805      JR      C,&0483          ; (+&05)
047E 77        LD      (HL),A
047F 23        INC     HL
0480 18F1      JR      &0473            ; (-&0f)
0482 E1        POP     HL
0483 DF        RST     &18
0484 3F        CCF     
0485 0D        DEC     C
0486 8A        ADC     A,D
0487 18D3      JR      &045C            ; (-&2d)
0489 CD3605    CALL    &0536
048C CF        RST     &08
048D A4        AND     H
048E C8        RET     Z

048F 1806      JR      &0497            ; (+&06)
0491 CD3605    CALL    &0536
0494 CF        RST     &08
0495 A5        AND     L
0496 C8        RET     Z

0497 DF        RST     &18
0498 44        LD      B,H
0499 69        LD      L,C
049A 73        LD      (HL),E
049B 63        LD      H,E
049C 3AA03A    LD      A,(&3AA0)
049F 56        LD      D,(HL)
04A0 FB        EI      
04A1 3D        DEC     A
04A2 2820      JR      Z,&04C4          ; (+&20)
04A4 3D        DEC     A
04A5 2828      JR      Z,&04CF          ; (+&28)
04A7 3D        DEC     A
04A8 2831      JR      Z,&04DB          ; (+&31)
04AA 3D        DEC     A
04AB 280B      JR      Z,&04B8          ; (+&0b)
04AD DF        RST     &18
04AE 4E        LD      C,(HL)
04AF 6F        LD      L,A
04B0 2044      JR      NZ,&04F6         ; (+&44)
04B2 72        LD      (HL),D
04B3 69        LD      L,C
04B4 76        HALT    
04B5 E5        PUSH    HL
04B6 182D      JR      &04E5            ; (+&2d)
04B8 DF        RST     &18
04B9 4E        LD      C,(HL)
04BA 6F        LD      L,A
04BB 74        LD      (HL),H
04BC 2052      JR      NZ,&0510         ; (+&52)
04BE 65        LD      H,L
04BF 61        LD      H,C
04C0 64        LD      H,H
04C1 F9        LD      SP,HL
04C2 1821      JR      &04E5            ; (+&21)
04C4 DF        RST     &18
04C5 42        LD      B,D
04C6 61        LD      H,C
04C7 64        LD      H,H
04C8 2044      JR      NZ,&050E         ; (+&44)
04CA 61        LD      H,C
04CB 74        LD      (HL),H
04CC E1        POP     HL
04CD 1816      JR      &04E5            ; (+&16)
04CF DF        RST     &18
04D0 50        LD      D,B
04D1 72        LD      (HL),D
04D2 6F        LD      L,A
04D3 74        LD      (HL),H
04D4 65        LD      H,L
04D5 63        LD      H,E
04D6 74        LD      (HL),H
04D7 65        LD      H,L
04D8 E4180A    CALL    PO,&0A18
04DB DF        RST     &18
04DC 4E        LD      C,(HL)
04DD 6F        LD      L,A
04DE 2053      JR      NZ,&0533         ; (+&53)
04E0 65        LD      H,L
04E1 63        LD      H,E
04E2 74        LD      (HL),H
04E3 6F        LD      L,A
04E4 F2C3FC    JP      P,&FCC3
04E7 01CD42    LD      BC,&42CD
04EA 05        DEC     B
04EB D5        PUSH    DE
04EC CF        RST     &08
04ED A2        AND     D
04EE D1        POP     DE
04EF 2015      JR      NZ,&0506         ; (+&15)
04F1 CD0B05    CALL    &050B
04F4 38F5      JR      C,&04EB          ; (-&0b)
04F6 180E      JR      &0506            ; (+&0e)
04F8 CD4205    CALL    &0542
04FB D5        PUSH    DE
04FC CF        RST     &08
04FD A3        AND     E
04FE D1        POP     DE
04FF 2005      JR      NZ,&0506         ; (+&05)
0501 CD0B05    CALL    &050B
0504 38F5      JR      C,&04FB          ; (-&0b)
0506 3A56FB    LD      A,(&FB56)
0509 B7        OR      A
050A C9        RET     

050B 2152FB    LD      HL,&FB52
050E 34        INC     (HL)
050F CDBE1D    CALL    &1DBE
0512 4F        LD      C,A
0513 7E        LD      A,(HL)
0514 B9        CP      C
0515 380E      JR      C,&0525          ; (+&0e)
0517 3600      LD      (HL),&00
0519 2B        DEC     HL
051A 34        INC     (HL)
051B 7E        LD      A,(HL)
051C FE28      CP      &28
051E 3805      JR      C,&0525          ; (+&05)
0520 E1        POP     HL
0521 3E03      LD      A,&03
0523 180D      JR      &0532            ; (+&0d)
0525 2A53FB    LD      HL,(&FB53)
0528 010002    LD      BC,&0200
052B 09        ADD     HL,BC
052C 2253FB    LD      (&FB53),HL
052F AF        XOR     A
0530 ED52      SBC     HL,DE
0532 3256FB    LD      (&FB56),A
0535 C9        RET     

0536 D9        EXX     
0537 CF        RST     &08
0538 B0        OR      B
0539 CF        RST     &08
053A AC        XOR     H
053B 7D        LD      A,L
053C D9        EXX     
053D FE04      CP      &04
053F D8        RET     C

0540 18A3      JR      &04E5            ; (-&5d)
0542 3250FB    LD      (&FB50),A
0545 2253FB    LD      (&FB53),HL
0548 ED4351FB  LD      (&FB51),BC
054C C9        RET     

054D 79        LD      A,C
054E B7        OR      A
054F 2002      JR      NZ,&0553         ; (+&02)
0551 3E08      LD      A,&08
0553 4F        LD      C,A
0554 CF        RST     &08
0555 9B        SBC     A,E
0556 FE1B      CP      &1B
0558 C8        RET     Z

0559 CF        RST     &08
055A A9        XOR     C
055B 41        LD      B,C
055C E5        PUSH    HL
055D CF        RST     &08
055E B1        OR      C
055F CF        RST     &08
0560 AA        XOR     D
0561 B7        OR      A
0562 ED52      SBC     HL,DE
0564 19        ADD     HL,DE
0565 23        INC     HL
0566 3002      JR      NC,&056A         ; (+&02)
0568 10F3      DJNZ    &055D            ; (-&0d)
056A E1        POP     HL
056B 41        LD      B,C
056C CF        RST     &08
056D B1        OR      C
056E E67F      AND     &7F
0570 FE20      CP      &20
0572 3804      JR      C,&0578          ; (+&04)
0574 FE7F      CP      &7F
0576 2002      JR      NZ,&057A         ; (+&02)
0578 3E2E      LD      A,&2E
057A D7        RST     &10
057B B7        OR      A
057C ED52      SBC     HL,DE
057E 19        ADD     HL,DE
057F 23        INC     HL
0580 D0        RET     NC

0581 10E9      DJNZ    &056C            ; (-&17)
0583 CF        RST     &08
0584 A6        AND     (HL)
0585 18CD      JR      &0554            ; (-&33)
0587 2A32FB    LD      HL,(&FB32)
058A 1803      JR      &058F            ; (+&03)
058C 2A34FB    LD      HL,(&FB34)
058F E5        PUSH    HL
0590 C317FB    JP      &FB17
0593 7D        LD      A,L
0594 FE02      CP      &02
0596 284B      JR      Z,&05E3          ; (+&4b)
0598 3F        CCF     
0599 D8        RET     C

059A B7        OR      A
059B 116FFB    LD      DE,&FB6F
059E 3E20      LD      A,&20
05A0 2805      JR      Z,&05A7          ; (+&05)
05A2 1167FB    LD      DE,&FB67
05A5 3E27      LD      A,&27
05A7 F5        PUSH    AF
05A8 CF        RST     &08
05A9 A7        AND     A
05AA F1        POP     AF
05AB D7        RST     &10
05AC F5        PUSH    AF
05AD DF        RST     &18
05AE 41        LD      B,C
05AF 2020      JR      NZ,&05D1         ; (+&20)
05B1 42        LD      B,D
05B2 43        LD      B,E
05B3 2020      JR      NZ,&05D5         ; (+&20)
05B5 2044      JR      NZ,&05FB         ; (+&44)
05B7 45        LD      B,L
05B8 2020      JR      NZ,&05DA         ; (+&20)
05BA 2048      JR      NZ,&0604         ; (+&48)
05BC 4C        LD      C,H
05BD 2020      JR      NZ,&05DF         ; (+&20)
05BF 2050      JR      NZ,&0611         ; (+&50)
05C1 43        LD      B,E
05C2 2020      JR      NZ,&05E4         ; (+&20)
05C4 53        LD      D,E
05C5 5A        LD      E,D
05C6 2D        DEC     L
05C7 48        LD      C,B
05C8 2D        DEC     L
05C9 50        LD      D,B
05CA 4E        LD      C,(HL)
05CB C3F1D7    JP      &D7F1
05CE CF        RST     &08
05CF A6        AND     (HL)
05D0 D5        PUSH    DE
05D1 CDFE05    CALL    &05FE
05D4 E1        POP     HL
05D5 4E        LD      C,(HL)
05D6 0608      LD      B,&08
05D8 3E18      LD      A,&18
05DA CB01      RLC     C
05DC 17        RLA     
05DD D7        RST     &10
05DE 1F        RRA     
05DF 10F9      DJNZ    &05DA            ; (-&07)
05E1 B7        OR      A
05E2 C9        RET     

05E3 115FFB    LD      DE,&FB5F
05E6 DF        RST     &18
05E7 2049      JR      NZ,&0632         ; (+&49)
05E9 2020      JR      NZ,&060B         ; (+&20)
05EB 49        LD      C,C
05EC 58        LD      E,B
05ED 2020      JR      NZ,&060F         ; (+&20)
05EF 2049      JR      NZ,&063A         ; (+&49)
05F1 59        LD      E,C
05F2 2020      JR      NZ,&0614         ; (+&20)
05F4 2053      JR      NZ,&0649         ; (+&53)
05F6 50        LD      D,B
05F7 2020      JR      NZ,&0619         ; (+&20)
05F9 2050      JR      NZ,&064B         ; (+&50)
05FB 43        LD      B,E
05FC 0D        DEC     C
05FD 8A        ADC     A,D
05FE CD080F    CALL    &0F08
0601 7C        LD      A,H
0602 CF        RST     &08
0603 AA        XOR     D
0604 0603      LD      B,&03
0606 CD080F    CALL    &0F08
0609 CF        RST     &08
060A A9        XOR     C
060B 10F9      DJNZ    &0606            ; (-&07)
060D 2A77FB    LD      HL,(&FB77)
0610 CF        RST     &08
0611 A9        XOR     C
0612 B7        OR      A
0613 C9        RET     

0614 CF        RST     &08
0615 9C        SBC     A,H
0616 D7        RST     &10
0617 FE0D      CP      &0D
0619 20F9      JR      NZ,&0614         ; (-&07)
061B C5        PUSH    BC
061C E5        PUSH    HL
061D D5        PUSH    DE
061E AF        XOR     A
061F CF        RST     &08
0620 D1        POP     DE
0621 E1        POP     HL
0622 E5        PUSH    HL
0623 3A45FB    LD      A,(&FB45)
0626 CB57      BIT     2,A
0628 3A4FFB    LD      A,(&FB4F)
062B 5F        LD      E,A
062C 57        LD      D,A
062D 2009      JR      NZ,&0638         ; (+&09)
062F EF        RST     &28
0630 77        LD      (HL),A
0631 23        INC     HL
0632 03        INC     BC
0633 1D        DEC     E
0634 20F9      JR      NZ,&062F         ; (-&07)
0636 180B      JR      &0643            ; (+&0b)
0638 CDE90B    CALL    &0BE9
063B 77        LD      (HL),A
063C 23        INC     HL
063D CDCF0B    CALL    &0BCF
0640 1D        DEC     E
0641 20F5      JR      NZ,&0638         ; (-&0b)
0643 3600      LD      (HL),&00
0645 15        DEC     D
0646 2806      JR      Z,&064E          ; (+&06)
0648 2B        DEC     HL
0649 7E        LD      A,(HL)
064A FE20      CP      &20
064C 28F5      JR      Z,&0643          ; (-&0b)
064E D1        POP     DE
064F 2140FB    LD      HL,&FB40
0652 1A        LD      A,(DE)
0653 E67F      AND     &7F
0655 BE        CP      (HL)
0656 2001      JR      NZ,&0659         ; (+&01)
0658 13        INC     DE
0659 DF        RST     &18
065A 8A        ADC     A,D
065B E1        POP     HL
065C C1        POP     BC
065D C9        RET     

065E 3A93FB    LD      A,(&FB93)
0661 B7        OR      A
0662 281B      JR      Z,&067F          ; (+&1b)
0664 C5        PUSH    BC
0665 ED4B94FB  LD      BC,(&FB94)
0669 EF        RST     &28
066A 0C        INC     C
066B CB7F      BIT     7,A
066D CBBF      RES     7,A
066F ED4394FB  LD      (&FB94),BC
0673 C1        POP     BC
0674 C8        RET     Z

0675 F5        PUSH    AF
0676 AF        XOR     A
0677 3293FB    LD      (&FB93),A
067A 32B2FB    LD      (&FBB2),A
067D F1        POP     AF
067E C9        RET     

067F E5        PUSH    HL
0680 D5        PUSH    DE
0681 C5        PUSH    BC
0682 3AB2FB    LD      A,(&FBB2)
0685 B7        OR      A
0686 F5        PUSH    AF
0687 2032      JR      NZ,&06BB         ; (+&32)
0689 F1        POP     AF
068A CF        RST     &08
068B D1        POP     DE
068C CF        RST     &08
068D C23247    JP      NZ,&4732
0690 FB        EI      
0691 CDB30A    CALL    &0AB3
0694 CDC406    CALL    &06C4
0697 2008      JR      NZ,&06A1         ; (+&08)
0699 CDA80A    CALL    &0AA8
069C CDC406    CALL    &06C4
069F 28EB      JR      Z,&068C          ; (-&15)
06A1 CDB30A    CALL    &0AB3
06A4 CF        RST     &08
06A5 9B        SBC     A,E
06A6 200F      JR      NZ,&06B7         ; (+&0f)
06A8 3A45FB    LD      A,(&FB45)
06AB CB57      BIT     2,A
06AD 28E2      JR      Z,&0691          ; (-&1e)
06AF 0600      LD      B,&00
06B1 E5        PUSH    HL
06B2 E1        POP     HL
06B3 10FC      DJNZ    &06B1            ; (-&04)
06B5 18DA      JR      &0691            ; (-&26)
06B7 F5        PUSH    AF
06B8 CDA80A    CALL    &0AA8
06BB AF        XOR     A
06BC 32B2FB    LD      (&FBB2),A
06BF F1        POP     AF
06C0 C1        POP     BC
06C1 D1        POP     DE
06C2 E1        POP     HL
06C3 C9        RET     

06C4 3A41FB    LD      A,(&FB41)
06C7 57        LD      D,A
06C8 1E00      LD      E,&00
06CA CD6307    CALL    &0763
06CD C0        RET     NZ

06CE 1B        DEC     DE
06CF 7A        LD      A,D
06D0 B3        OR      E
06D1 20F7      JR      NZ,&06CA         ; (-&09)
06D3 213EFB    LD      HL,&FB3E
06D6 C9        RET     

06D7 E5        PUSH    HL
06D8 D5        PUSH    DE
06D9 C5        PUSH    BC
06DA CD6307    CALL    &0763
06DD CA5407    JP      Z,&0754
06E0 CF        RST     &08
06E1 B5        OR      L
06E2 34        INC     (HL)
06E3 35        DEC     (HL)
06E4 2818      JR      Z,&06FE          ; (+&18)
06E6 B7        OR      A
06E7 200C      JR      NZ,&06F5         ; (+&0c)
06E9 213EFB    LD      HL,&FB3E
06EC CB5E      BIT     3,(HL)
06EE 2146FB    LD      HL,&FB46
06F1 2061      JR      NZ,&0754         ; (+&61)
06F3 1809      JR      &06FE            ; (+&09)
06F5 C5        PUSH    BC
06F6 47        LD      B,A
06F7 96        SUB     (HL)
06F8 E60F      AND     &0F
06FA 78        LD      A,B
06FB C1        POP     BC
06FC 280A      JR      Z,&0708          ; (+&0a)
06FE F5        PUSH    AF
06FF 3A42FB    LD      A,(&FB42)
0702 32B5FB    LD      (&FBB5),A
0705 F1        POP     AF
0706 181C      JR      &0724            ; (+&1c)
0708 21B5FB    LD      HL,&FBB5
070B 35        DEC     (HL)
070C 200E      JR      NZ,&071C         ; (+&0e)
070E AF        XOR     A
070F 3246FB    LD      (&FB46),A
0712 3A43FB    LD      A,(&FB43)
0715 32B5FB    LD      (&FBB5),A
0718 CF        RST     &08
0719 B5        OR      L
071A 2008      JR      NZ,&0724         ; (+&08)
071C 213EFB    LD      HL,&FB3E
071F CB86      RES     0,(HL)
0721 AF        XOR     A
0722 1830      JR      &0754            ; (+&30)
0724 77        LD      (HL),A
0725 213EFB    LD      HL,&FB3E
0728 CB4E      BIT     1,(HL)
072A 2828      JR      Z,&0754          ; (+&28)
072C 5F        LD      E,A
072D CBBB      RES     7,E
072F 01803B    LD      BC,&3B80
0732 1C        INC     E
0733 1D        DEC     E
0734 280D      JR      Z,&0743          ; (+&0d)
0736 AF        XOR     A
0737 0C        INC     C
0738 0D        DEC     C
0739 2819      JR      Z,&0754          ; (+&19)
073B EF        RST     &28
073C 0C        INC     C
073D CB7F      BIT     7,A
073F 28F5      JR      Z,&0736          ; (-&0b)
0741 18F0      JR      &0733            ; (-&10)
0743 EF        RST     &28
0744 0C        INC     C
0745 CB7F      BIT     7,A
0747 CBBF      RES     7,A
0749 ED4394FB  LD      (&FB94),BC
074D 2005      JR      NZ,&0754         ; (+&05)
074F 2193FB    LD      HL,&FB93
0752 3601      LD      (HL),&01
0754 B7        OR      A
0755 213EFB    LD      HL,&FB3E
0758 CB46      BIT     0,(HL)
075A CB86      RES     0,(HL)
075C 32B2FB    LD      (&FBB2),A
075F C1        POP     BC
0760 D1        POP     DE
0761 E1        POP     HL
0762 C9        RET     

0763 D5        PUSH    DE
0764 1EFF      LD      E,&FF
0766 CDA20F    CALL    &0FA2
0769 D1        POP     DE
076A C0        RET     NZ

076B 3246FB    LD      (&FB46),A
076E C9        RET     

076F 3A3EFB    LD      A,(&FB3E)
0772 E680      AND     &80
0774 323EFB    LD      (&FB3E),A
0777 1E01      LD      E,&01
0779 CDA20F    CALL    &0FA2
077C 1F        RRA     
077D 47        LD      B,A
077E 301D      JR      NC,&079D         ; (+&1d)
0780 CDA50F    CALL    &0FA5
0783 1F        RRA     
0784 38FA      JR      C,&0780          ; (-&06)
0786 CDA80A    CALL    &0AA8
0789 DB20      IN      A,(&20)
078B 07        RLCA    
078C 3809      JR      C,&0797          ; (+&09)
078E F1        POP     AF
078F ED5B30FB  LD      DE,(&FB30)
0793 D5        PUSH    DE
0794 C317FB    JP      &FB17
0797 07        RLCA    
0798 D25002    JP      NC,&0250
079B 183F      JR      &07DC            ; (+&3f)
079D 213EFB    LD      HL,&FB3E
07A0 DB20      IN      A,(&20)
07A2 2F        CPL     
07A3 E6E0      AND     &E0
07A5 0F        RRCA    
07A6 B6        OR      (HL)
07A7 77        LD      (HL),A
07A8 78        LD      A,B
07A9 CB3F      SRL     A
07AB 282F      JR      Z,&07DC          ; (+&2f)
07AD 1F        RRA     
07AE 3002      JR      NC,&07B2         ; (+&02)
07B0 0680      LD      B,&80
07B2 1F        RRA     
07B3 3002      JR      NC,&07B7         ; (+&02)
07B5 0687      LD      B,&87
07B7 1F        RRA     
07B8 300F      JR      NC,&07C9         ; (+&0f)
07BA 7E        LD      A,(HL)
07BB EE80      XOR     &80
07BD 77        LD      (HL),A
07BE DB22      IN      A,(&22)
07C0 CDA50F    CALL    &0FA5
07C3 CB67      BIT     4,A
07C5 20F9      JR      NZ,&07C0         ; (-&07)
07C7 1813      JR      &07DC            ; (+&13)
07C9 1F        RRA     
07CA 3002      JR      NC,&07CE         ; (+&02)
07CC 060D      LD      B,&0D
07CE 0F        RRCA    
07CF 3002      JR      NC,&07D3         ; (+&02)
07D1 0620      LD      B,&20
07D3 1F        RRA     
07D4 3002      JR      NC,&07D8         ; (+&02)
07D6 061B      LD      B,&1B
07D8 CBC6      SET     0,(HL)
07DA CBDE      SET     3,(HL)
07DC 110100    LD      DE,&0001
07DF 0EFF      LD      C,&FF
07E1 CB03      RLC     E
07E3 CDA20F    CALL    &0FA2
07E6 2878      JR      Z,&0860          ; (+&78)
07E8 0C        INC     C
07E9 0F        RRCA    
07EA 3248FB    LD      (&FB48),A
07ED 3069      JR      NC,&0858         ; (+&69)
07EF 213EFB    LD      HL,&FB3E
07F2 CB5E      BIT     3,(HL)
07F4 C28308    JP      NZ,&0883
07F7 C5        PUSH    BC
07F8 7A        LD      A,D
07F9 87        ADD     A,A
07FA 87        ADD     A,A
07FB 87        ADD     A,A
07FC 47        LD      B,A
07FD 87        ADD     A,A
07FE 80        ADD     A,B
07FF 0600      LD      B,&00
0801 21C510    LD      HL,&10C5
0804 09        ADD     HL,BC
0805 4F        LD      C,A
0806 09        ADD     HL,BC
0807 3A3EFB    LD      A,(&FB3E)
080A CB7F      BIT     7,A
080C 2816      JR      Z,&0824          ; (+&16)
080E CB67      BIT     4,A
0810 2012      JR      NZ,&0824         ; (+&12)
0812 7E        LD      A,(HL)
0813 FE61      CP      &61
0815 380D      JR      C,&0824          ; (+&0d)
0817 FE7B      CP      &7B
0819 3009      JR      NC,&0824         ; (+&09)
081B E5        PUSH    HL
081C 213EFB    LD      HL,&FB3E
081F CBD6      SET     2,(HL)
0821 E1        POP     HL
0822 1807      JR      &082B            ; (+&07)
0824 3A3EFB    LD      A,(&FB3E)
0827 CB77      BIT     6,A
0829 2804      JR      Z,&082F          ; (+&04)
082B 010800    LD      BC,&0008
082E 09        ADD     HL,BC
082F C1        POP     BC
0830 3A3EFB    LD      A,(&FB3E)
0833 CB6F      BIT     5,A
0835 281B      JR      Z,&0852          ; (+&1b)
0837 CB77      BIT     6,A
0839 2048      JR      NZ,&0883         ; (+&48)
083B C5        PUSH    BC
083C 010800    LD      BC,&0008
083F 09        ADD     HL,BC
0840 3A3EFB    LD      A,(&FB3E)
0843 CB57      BIT     2,A
0845 3A48FB    LD      A,(&FB48)
0848 2001      JR      NZ,&084B         ; (+&01)
084A 09        ADD     HL,BC
084B C1        POP     BC
084C 3248FB    LD      (&FB48),A
084F 3A3EFB    LD      A,(&FB3E)
0852 F609      OR      &09
0854 323EFB    LD      (&FB3E),A
0857 46        LD      B,(HL)
0858 79        LD      A,C
0859 FE07      CP      &07
085B 3A48FB    LD      A,(&FB48)
085E 2088      JR      NZ,&07E8         ; (-&78)
0860 14        INC     D
0861 7A        LD      A,D
0862 FE07      CP      &07
0864 C2DF07    JP      NZ,&07DF
0867 CD970F    CALL    &0F97
086A 213EFB    LD      HL,&FB3E
086D CB78      BIT     7,B
086F 280A      JR      Z,&087B          ; (+&0a)
0871 CBCE      SET     1,(HL)
0873 CB76      BIT     6,(HL)
0875 2816      JR      Z,&088D          ; (+&16)
0877 CBD8      SET     3,B
0879 1812      JR      &088D            ; (+&12)
087B CB66      BIT     4,(HL)
087D 280E      JR      Z,&088D          ; (+&0e)
087F CBF8      SET     7,B
0881 180A      JR      &088D            ; (+&0a)
0883 213EFB    LD      HL,&FB3E
0886 CB86      RES     0,(HL)
0888 CD970F    CALL    &0F97
088B AF        XOR     A
088C 47        LD      B,A
088D CB46      BIT     0,(HL)
088F 2146FB    LD      HL,&FB46
0892 C8        RET     Z

0893 78        LD      A,B
0894 BE        CP      (HL)
0895 C9        RET     

0896 2145FB    LD      HL,&FB45
0899 CB5E      BIT     3,(HL)
089B C24E0A    JP      NZ,&0A4E
089E CB4E      BIT     1,(HL)
08A0 C45C0F    CALL    NZ,&0F5C
08A3 FE20      CP      &20
08A5 3815      JR      C,&08BC          ; (+&15)
08A7 2145FB    LD      HL,&FB45
08AA CB56      BIT     2,(HL)
08AC 2806      JR      Z,&08B4          ; (+&06)
08AE CB7E      BIT     7,(HL)
08B0 2802      JR      Z,&08B4          ; (+&02)
08B2 EE80      XOR     &80
08B4 214BFB    LD      HL,&FB4B
08B7 CDCE0A    CALL    &0ACE
08BA 1813      JR      &08CF            ; (+&13)
08BC 87        ADD     A,A
08BD 6F        LD      L,A
08BE 2600      LD      H,&00
08C0 11680A    LD      DE,&0A68
08C3 19        ADD     HL,DE
08C4 CD090F    CALL    &0F09
08C7 E5        PUSH    HL
08C8 214BFB    LD      HL,&FB4B
08CB 3A45FB    LD      A,(&FB45)
08CE C9        RET     

08CF 2B        DEC     HL
08D0 34        INC     (HL)
08D1 3A4FFB    LD      A,(&FB4F)
08D4 BE        CP      (HL)
08D5 C0        RET     NZ

08D6 3600      LD      (HL),&00
08D8 23        INC     HL
08D9 3E18      LD      A,&18
08DB 3D        DEC     A
08DC BE        CP      (HL)
08DD CAB30C    JP      Z,&0CB3
08E0 34        INC     (HL)
08E1 C9        RET     

08E2 7E        LD      A,(HL)
08E3 B7        OR      A
08E4 C8        RET     Z

08E5 35        DEC     (HL)
08E6 C9        RET     

08E7 2B        DEC     HL
08E8 7E        LD      A,(HL)
08E9 B7        OR      A
08EA 20F9      JR      NZ,&08E5         ; (-&07)
08EC 3A4FFB    LD      A,(&FB4F)
08EF 3D        DEC     A
08F0 77        LD      (HL),A
08F1 23        INC     HL
08F2 35        DEC     (HL)
08F3 F0        RET     P

08F4 3600      LD      (HL),&00
08F6 2B        DEC     HL
08F7 3600      LD      (HL),&00
08F9 C9        RET     

08FA 5F        LD      E,A
08FB CD310E    CALL    &0E31
08FE C0        RET     NZ

08FF 7B        LD      A,E
0900 0E50      LD      C,&50
0902 110000    LD      DE,&0000
0905 CBD7      SET     2,A
0907 21A010    LD      HL,&10A0
090A 1814      JR      &0920            ; (+&14)
090C 0E28      LD      C,&28
090E 119803    LD      DE,&0398
0911 21A010    LD      HL,&10A0
0914 1808      JR      &091E            ; (+&08)
0916 0E20      LD      C,&20
0918 11E002    LD      DE,&02E0
091B 21A81C    LD      HL,&1CA8
091E CB97      RES     2,A
0920 CBBF      RES     7,A
0922 3245FB    LD      (&FB45),A
0925 2242FB    LD      (&FB42),HL
0928 214FFB    LD      HL,&FB4F
092B 71        LD      (HL),C
092C ED534CFB  LD      (&FB4C),DE
0930 CB57      BIT     2,A
0932 201E      JR      NZ,&0952         ; (+&1e)
0934 01003C    LD      BC,&3C00
0937 E7        RST     &20
0938 01C003    LD      BC,&03C0
093B 1E20      LD      E,&20
093D CDA60C    CALL    &0CA6
0940 E7        RST     &20
0941 0618      LD      B,&18
0943 CDAF0C    CALL    &0CAF
0946 0620      LD      B,&20
0948 E7        RST     &20
0949 CD9F0C    CALL    &0C9F
094C AF        XOR     A
094D CDB00D    CALL    &0DB0
0950 1815      JR      &0967            ; (+&15)
0952 014000    LD      BC,&0040
0955 3E20      LD      A,&20
0957 ED79      OUT     (C),A
0959 10FC      DJNZ    &0957            ; (-&04)
095B 0C        INC     C
095C 3E48      LD      A,&48
095E B9        CP      C
095F 20F4      JR      NZ,&0955         ; (-&0c)
0961 210000    LD      HL,&0000
0964 CD300D    CALL    &0D30
0967 214BFB    LD      HL,&FB4B
096A 1888      JR      &08F4            ; (-&78)
096C 2B        DEC     HL
096D 3600      LD      (HL),&00
096F 23        INC     HL
0970 2B        DEC     HL
0971 E5        PUSH    HL
0972 2A4AFB    LD      HL,(&FB4A)
0975 E3        EX      (SP),HL
0976 3E20      LD      A,&20
0978 CDCE0A    CALL    &0ACE
097B 3A4FFB    LD      A,(&FB4F)
097E 34        INC     (HL)
097F BE        CP      (HL)
0980 20F4      JR      NZ,&0976         ; (-&0c)
0982 E1        POP     HL
0983 224AFB    LD      (&FB4A),HL
0986 C9        RET     

0987 2B        DEC     HL
0988 E5        PUSH    HL
0989 CD7109    CALL    &0971
098C E3        EX      (SP),HL
098D 3600      LD      (HL),&00
098F 3E18      LD      A,&18
0991 23        INC     HL
0992 34        INC     (HL)
0993 96        SUB     (HL)
0994 28EC      JR      Z,&0982          ; (-&14)
0996 2B        DEC     HL
0997 E5        PUSH    HL
0998 CD7109    CALL    &0971
099B E1        POP     HL
099C 18F1      JR      &098F            ; (-&0f)
099E 7E        LD      A,(HL)
099F 2B        DEC     HL
09A0 B6        OR      (HL)
09A1 C8        RET     Z

09A2 3E08      LD      A,&08
09A4 D7        RST     &10
09A5 23        INC     HL
09A6 2B        DEC     HL
09A7 E5        PUSH    HL
09A8 CF        RST     &08
09A9 D1        POP     DE
09AA 2A4AFB    LD      HL,(&FB4A)
09AD E3        EX      (SP),HL
09AE 3A4FFB    LD      A,(&FB4F)
09B1 3D        DEC     A
09B2 BE        CP      (HL)
09B3 2814      JR      Z,&09C9          ; (+&14)
09B5 03        INC     BC
09B6 3A45FB    LD      A,(&FB45)
09B9 CB57      BIT     2,A
09BB 2804      JR      Z,&09C1          ; (+&04)
09BD 0B        DEC     BC
09BE CDCF0B    CALL    &0BCF
09C1 CF        RST     &08
09C2 C2CDCE    JP      NZ,&CECD
09C5 0A        LD      A,(BC)
09C6 34        INC     (HL)
09C7 18E5      JR      &09AE            ; (-&1b)
09C9 3E20      LD      A,&20
09CB CDCE0A    CALL    &0ACE
09CE 18B2      JR      &0982            ; (-&4e)
09D0 2B        DEC     HL
09D1 E5        PUSH    HL
09D2 CF        RST     &08
09D3 D1        POP     DE
09D4 2A4AFB    LD      HL,(&FB4A)
09D7 E3        EX      (SP),HL
09D8 1E20      LD      E,&20
09DA CF        RST     &08
09DB C203F5    JP      NZ,&F503
09DE 3A45FB    LD      A,(&FB45)
09E1 CB57      BIT     2,A
09E3 2804      JR      Z,&09E9          ; (+&04)
09E5 0B        DEC     BC
09E6 CDCF0B    CALL    &0BCF
09E9 3A4FFB    LD      A,(&FB4F)
09EC BE        CP      (HL)
09ED 2809      JR      Z,&09F8          ; (+&09)
09EF 7B        LD      A,E
09F0 CDCE0A    CALL    &0ACE
09F3 34        INC     (HL)
09F4 F1        POP     AF
09F5 5F        LD      E,A
09F6 18E2      JR      &09DA            ; (-&1e)
09F8 F1        POP     AF
09F9 1887      JR      &0982            ; (-&79)
09FB 2A4AFB    LD      HL,(&FB4A)
09FE E5        PUSH    HL
09FF AF        XOR     A
0A00 47        LD      B,A
0A01 4F        LD      C,A
0A02 CD270A    CALL    &0A27
0A05 ED434AFB  LD      (&FB4A),BC
0A09 C5        PUSH    BC
0A0A CF        RST     &08
0A0B D1        POP     DE
0A0C CF        RST     &08
0A0D C2CF9F    JP      NZ,&9FCF
0A10 C1        POP     BC
0A11 ED5B4FFB  LD      DE,(&FB4F)
0A15 1618      LD      D,&18
0A17 0C        INC     C
0A18 79        LD      A,C
0A19 BB        CP      E
0A1A 20E9      JR      NZ,&0A05         ; (-&17)
0A1C 0E00      LD      C,&00
0A1E 04        INC     B
0A1F 78        LD      A,B
0A20 BA        CP      D
0A21 20DF      JR      NZ,&0A02         ; (-&21)
0A23 E1        POP     HL
0A24 224AFB    LD      (&FB4A),HL
0A27 F5        PUSH    AF
0A28 3E0D      LD      A,&0D
0A2A CF        RST     &08
0A2B 9F        SBC     A,A
0A2C 3E0A      LD      A,&0A
0A2E CF        RST     &08
0A2F 9F        SBC     A,A
0A30 F1        POP     AF
0A31 C9        RET     

0A32 EE80      XOR     &80
0A34 180E      JR      &0A44            ; (+&0e)
0A36 CBCF      SET     1,A
0A38 180A      JR      &0A44            ; (+&0a)
0A3A CB87      RES     0,A
0A3C 1806      JR      &0A44            ; (+&06)
0A3E CBC7      SET     0,A
0A40 1802      JR      &0A44            ; (+&02)
0A42 CB8F      RES     1,A
0A44 3245FB    LD      (&FB45),A
0A47 C9        RET     

0A48 2145FB    LD      HL,&FB45
0A4B CBDE      SET     3,(HL)
0A4D C9        RET     

0A4E CB66      BIT     4,(HL)
0A50 200B      JR      NZ,&0A5D         ; (+&0b)
0A52 CBE6      SET     4,(HL)
0A54 214FFB    LD      HL,&FB4F
0A57 BE        CP      (HL)
0A58 D0        RET     NC

0A59 324AFB    LD      (&FB4A),A
0A5C C9        RET     

0A5D CB9E      RES     3,(HL)
0A5F CBA6      RES     4,(HL)
0A61 FE18      CP      &18
0A63 D0        RET     NC

0A64 324BFB    LD      (&FB4B),A
0A67 C9        RET     

0A68 47        LD      B,A
0A69 0A        LD      A,(BC)
0A6A FB        EI      
0A6B 09        ADD     HL,BC
0A6C 47        LD      B,A
0A6D 0A        LD      A,(BC)
0A6E 47        LD      B,A
0A6F 0A        LD      A,(BC)
0A70 CF        RST     &08
0A71 08        EX      AF,AF'
0A72 47        LD      B,A
0A73 0A        LD      A,(BC)
0A74 A6        AND     (HL)
0A75 09        ADD     HL,BC
0A76 71        LD      (HL),C
0A77 0F        RRCA    
0A78 E7        RST     &20
0A79 08        EX      AF,AF'
0A7A 47        LD      B,A
0A7B 0A        LD      A,(BC)
0A7C D9        EXX     
0A7D 08        EX      AF,AF'
0A7E E20830    JP      PO,&3008
0A81 09        ADD     HL,BC
0A82 F608      OR      &08
0A84 0C        INC     C
0A85 09        ADD     HL,BC
0A86 1609      LD      D,&09
0A88 FA083E    JP      M,&3E08
0A8B 0A        LD      A,(BC)
0A8C 360A      LD      (HL),&0A
0A8E 42        LD      B,D
0A8F 0A        LD      A,(BC)
0A90 3A0A70    LD      A,(&700A)
0A93 09        ADD     HL,BC
0A94 87        ADD     A,A
0A95 09        ADD     HL,BC
0A96 320A6C    LD      (&6C0A),A
0A99 09        ADD     HL,BC
0A9A 9E        SBC     A,(HL)
0A9B 09        ADD     HL,BC
0A9C D0        RET     NC

0A9D 09        ADD     HL,BC
0A9E 47        LD      B,A
0A9F 0A        LD      A,(BC)
0AA0 47        LD      B,A
0AA1 0A        LD      A,(BC)
0AA2 48        LD      C,B
0AA3 0A        LD      A,(BC)
0AA4 F40847    CALL    P,&4708
0AA7 0A        LD      A,(BC)
0AA8 3A45FB    LD      A,(&FB45)
0AAB CB57      BIT     2,A
0AAD 281C      JR      Z,&0ACB          ; (+&1c)
0AAF 1E20      LD      E,&20
0AB1 180B      JR      &0ABE            ; (+&0b)
0AB3 3A45FB    LD      A,(&FB45)
0AB6 CB57      BIT     2,A
0AB8 1F        RRA     
0AB9 280B      JR      Z,&0AC6          ; (+&0b)
0ABB D0        RET     NC

0ABC 1E00      LD      E,&00
0ABE 3E0A      LD      A,&0A
0AC0 D348      OUT     (&48),A
0AC2 7B        LD      A,E
0AC3 D349      OUT     (&49),A
0AC5 C9        RET     

0AC6 3A3FFB    LD      A,(&FB3F)
0AC9 3803      JR      C,&0ACE          ; (+&03)
0ACB 3A47FB    LD      A,(&FB47)
0ACE E5        PUSH    HL
0ACF D5        PUSH    DE
0AD0 C5        PUSH    BC
0AD1 47        LD      B,A
0AD2 3A45FB    LD      A,(&FB45)
0AD5 CB57      BIT     2,A
0AD7 202D      JR      NZ,&0B06         ; (+&2d)
0AD9 3A4FFB    LD      A,(&FB4F)
0ADC FE28      CP      &28
0ADE 78        LD      A,B
0ADF 282F      JR      Z,&0B10          ; (+&2f)
0AE1 CD4C0C    CALL    &0C4C
0AE4 F7        RST     &30
0AE5 E5        PUSH    HL
0AE6 210800    LD      HL,&0008
0AE9 C1        POP     BC
0AEA E5        PUSH    HL
0AEB D5        PUSH    DE
0AEC 55        LD      D,L
0AED CD900D    CALL    &0D90
0AF0 45        LD      B,L
0AF1 2145FB    LD      HL,&FB45
0AF4 CB7E      BIT     7,(HL)
0AF6 2809      JR      Z,&0B01          ; (+&09)
0AF8 21E0FB    LD      HL,&FBE0
0AFB 7E        LD      A,(HL)
0AFC 2F        CPL     
0AFD 77        LD      (HL),A
0AFE 23        INC     HL
0AFF 10FA      DJNZ    &0AFB            ; (-&06)
0B01 C1        POP     BC
0B02 E1        POP     HL
0B03 C3B00B    JP      &0BB0
0B06 78        LD      A,B
0B07 CD130C    CALL    &0C13
0B0A CDD90B    CALL    &0BD9
0B0D C3C20B    JP      &0BC2
0B10 ED4B4AFB  LD      BC,(&FB4A)
0B14 59        LD      E,C
0B15 F5        PUSH    AF
0B16 79        LD      A,C
0B17 E603      AND     &03
0B19 CB39      SRL     C
0B1B CB39      SRL     C
0B1D 08        EX      AF,AF'
0B1E 79        LD      A,C
0B1F 87        ADD     A,A
0B20 81        ADD     A,C
0B21 4F        LD      C,A
0B22 F1        POP     AF
0B23 CD510C    CALL    &0C51
0B26 F7        RST     &30
0B27 08        EX      AF,AF'
0B28 010800    LD      BC,&0008
0B2B CB4F      BIT     1,A
0B2D EB        EX      DE,HL
0B2E 2801      JR      Z,&0B31          ; (+&01)
0B30 09        ADD     HL,BC
0B31 FE03      CP      &03
0B33 282D      JR      Z,&0B62          ; (+&2d)
0B35 F5        PUSH    AF
0B36 4D        LD      C,L
0B37 44        LD      B,H
0B38 D5        PUSH    DE
0B39 1610      LD      D,&10
0B3B CD900D    CALL    &0D90
0B3E C1        POP     BC
0B3F F1        POP     AF
0B40 E5        PUSH    HL
0B41 21E0FB    LD      HL,&FBE0
0B44 1608      LD      D,&08
0B46 B7        OR      A
0B47 283D      JR      Z,&0B86          ; (+&3d)
0B49 3D        DEC     A
0B4A 284F      JR      Z,&0B9B          ; (+&4f)
0B4C CDC60B    CALL    &0BC6
0B4F E6FC      AND     &FC
0B51 C5        PUSH    BC
0B52 013FF0    LD      BC,&F03F
0B55 07        RLCA    
0B56 07        RLCA    
0B57 CDFA0B    CALL    &0BFA
0B5A C1        POP     BC
0B5B 23        INC     HL
0B5C 03        INC     BC
0B5D 15        DEC     D
0B5E 20EC      JR      NZ,&0B4C         ; (-&14)
0B60 184A      JR      &0BAC            ; (+&4a)
0B62 09        ADD     HL,BC
0B63 44        LD      B,H
0B64 4D        LD      C,L
0B65 D5        PUSH    DE
0B66 1608      LD      D,&08
0B68 CD900D    CALL    &0D90
0B6B C1        POP     BC
0B6C E5        PUSH    HL
0B6D 21E0FB    LD      HL,&FBE0
0B70 1608      LD      D,&08
0B72 7E        LD      A,(HL)
0B73 E6C0      AND     &C0
0B75 5F        LD      E,A
0B76 CDC60B    CALL    &0BC6
0B79 CB3F      SRL     A
0B7B CB3F      SRL     A
0B7D B3        OR      E
0B7E 77        LD      (HL),A
0B7F 23        INC     HL
0B80 03        INC     BC
0B81 15        DEC     D
0B82 20EE      JR      NZ,&0B72         ; (-&12)
0B84 1810      JR      &0B96            ; (+&10)
0B86 CDC60B    CALL    &0BC6
0B89 E6FC      AND     &FC
0B8B 5F        LD      E,A
0B8C 7E        LD      A,(HL)
0B8D E603      AND     &03
0B8F B3        OR      E
0B90 77        LD      (HL),A
0B91 23        INC     HL
0B92 03        INC     BC
0B93 15        DEC     D
0B94 20F0      JR      NZ,&0B86         ; (-&10)
0B96 210800    LD      HL,&0008
0B99 1814      JR      &0BAF            ; (+&14)
0B9B CDC60B    CALL    &0BC6
0B9E C5        PUSH    BC
0B9F 010FFC    LD      BC,&FC0F
0BA2 A0        AND     B
0BA3 CDFA0B    CALL    &0BFA
0BA6 C1        POP     BC
0BA7 23        INC     HL
0BA8 03        INC     BC
0BA9 15        DEC     D
0BAA 20EF      JR      NZ,&0B9B         ; (-&11)
0BAC 211000    LD      HL,&0010
0BAF C1        POP     BC
0BB0 E5        PUSH    HL
0BB1 C5        PUSH    BC
0BB2 55        LD      D,L
0BB3 CDA10D    CALL    &0DA1
0BB6 C1        POP     BC
0BB7 CBE8      SET     5,B
0BB9 E7        RST     &20
0BBA C1        POP     BC
0BBB 3A38FB    LD      A,(&FB38)
0BBE 5F        LD      E,A
0BBF CDA60C    CALL    &0CA6
0BC2 C1        POP     BC
0BC3 D1        POP     DE
0BC4 E1        POP     HL
0BC5 C9        RET     

0BC6 3A45FB    LD      A,(&FB45)
0BC9 CB7F      BIT     7,A
0BCB EF        RST     &28
0BCC C8        RET     Z

0BCD 2F        CPL     
0BCE C9        RET     

0BCF 04        INC     B
0BD0 C0        RET     NZ

0BD1 0C        INC     C
0BD2 CB99      RES     3,C
0BD4 CBA1      RES     4,C
0BD6 CBA9      RES     5,C
0BD8 C9        RET     

0BD9 F5        PUSH    AF
0BDA CDEF0B    CALL    &0BEF
0BDD F1        POP     AF
0BDE ED79      OUT     (C),A
0BE0 C9        RET     

0BE1 3A45FB    LD      A,(&FB45)
0BE4 CB57      BIT     2,A
0BE6 CA2800    JP      Z,&0028
0BE9 CDEF0B    CALL    &0BEF
0BEC ED78      IN      A,(C)
0BEE C9        RET     

0BEF DB4C      IN      A,(&4C)
0BF1 1F        RRA     
0BF2 30FB      JR      NC,&0BEF         ; (-&05)
0BF4 DB4C      IN      A,(&4C)
0BF6 1F        RRA     
0BF7 38FB      JR      C,&0BF4          ; (-&05)
0BF9 C9        RET     

0BFA D5        PUSH    DE
0BFB 07        RLCA    
0BFC 07        RLCA    
0BFD F5        PUSH    AF
0BFE E60F      AND     &0F
0C00 57        LD      D,A
0C01 F1        POP     AF
0C02 E6F0      AND     &F0
0C04 5F        LD      E,A
0C05 7E        LD      A,(HL)
0C06 A0        AND     B
0C07 B2        OR      D
0C08 77        LD      (HL),A
0C09 CBDD      SET     3,L
0C0B 7E        LD      A,(HL)
0C0C A1        AND     C
0C0D B3        OR      E
0C0E 77        LD      (HL),A
0C0F CB9D      RES     3,L
0C11 D1        POP     DE
0C12 C9        RET     

0C13 ED4B4AFB  LD      BC,(&FB4A)
0C17 F5        PUSH    AF
0C18 2600      LD      H,&00
0C1A 68        LD      L,B
0C1B 44        LD      B,H
0C1C 29        ADD     HL,HL
0C1D 29        ADD     HL,HL
0C1E 29        ADD     HL,HL
0C1F 29        ADD     HL,HL
0C20 54        LD      D,H
0C21 5D        LD      E,L
0C22 29        ADD     HL,HL
0C23 29        ADD     HL,HL
0C24 19        ADD     HL,DE
0C25 09        ADD     HL,BC
0C26 ED5B4CFB  LD      DE,(&FB4C)
0C2A 19        ADD     HL,DE
0C2B 7C        LD      A,H
0C2C E63F      AND     &3F
0C2E 67        LD      H,A
0C2F 3E0E      LD      A,&0E
0C31 D348      OUT     (&48),A
0C33 0E49      LD      C,&49
0C35 ED61      OUT     (C),H
0C37 3C        INC     A
0C38 D348      OUT     (&48),A
0C3A ED69      OUT     (C),L
0C3C 7C        LD      A,H
0C3D E607      AND     &07
0C3F 45        LD      B,L
0C40 4F        LD      C,A
0C41 CBF1      SET     6,C
0C43 F1        POP     AF
0C44 C9        RET     

0C45 2145FB    LD      HL,&FB45
0C48 CB56      BIT     2,(HL)
0C4A 20C7      JR      NZ,&0C13         ; (-&39)
0C4C ED4B4AFB  LD      BC,(&FB4A)
0C50 59        LD      E,C
0C51 F5        PUSH    AF
0C52 2600      LD      H,&00
0C54 6F        LD      L,A
0C55 29        ADD     HL,HL
0C56 29        ADD     HL,HL
0C57 29        ADD     HL,HL
0C58 3E18      LD      A,&18
0C5A 84        ADD     A,H
0C5B 67        LD      H,A
0C5C E5        PUSH    HL
0C5D 2600      LD      H,&00
0C5F 68        LD      L,B
0C60 29        ADD     HL,HL
0C61 29        ADD     HL,HL
0C62 29        ADD     HL,HL
0C63 E5        PUSH    HL
0C64 29        ADD     HL,HL
0C65 29        ADD     HL,HL
0C66 E5        PUSH    HL
0C67 0600      LD      B,&00
0C69 09        ADD     HL,BC
0C6A 010038    LD      BC,&3800
0C6D E5        PUSH    HL
0C6E 09        ADD     HL,BC
0C6F E3        EX      (SP),HL
0C70 C1        POP     BC
0C71 EF        RST     &28
0C72 6F        LD      L,A
0C73 29        ADD     HL,HL
0C74 29        ADD     HL,HL
0C75 29        ADD     HL,HL
0C76 C1        POP     BC
0C77 E3        EX      (SP),HL
0C78 EB        EX      DE,HL
0C79 2600      LD      H,&00
0C7B 3A4FFB    LD      A,(&FB4F)
0C7E FE28      CP      &28
0C80 2001      JR      NZ,&0C83         ; (+&01)
0C82 19        ADD     HL,DE
0C83 09        ADD     HL,BC
0C84 01003C    LD      BC,&3C00
0C87 09        ADD     HL,BC
0C88 D1        POP     DE
0C89 E3        EX      (SP),HL
0C8A C1        POP     BC
0C8B F1        POP     AF
0C8C C9        RET     

0C8D F5        PUSH    AF
0C8E 79        LD      A,C
0C8F D309      OUT     (&09),A
0C91 F5        PUSH    AF
0C92 F1        POP     AF
0C93 78        LD      A,B
0C94 D309      OUT     (&09),A
0C96 F1        POP     AF
0C97 C9        RET     

0C98 CBF7      SET     6,A
0C9A D309      OUT     (&09),A
0C9C 00        NOP     
0C9D F1        POP     AF
0C9E C9        RET     

0C9F 010018    LD      BC,&1800
0CA2 3A38FB    LD      A,(&FB38)
0CA5 5F        LD      E,A
0CA6 7B        LD      A,E
0CA7 D308      OUT     (&08),A
0CA9 0B        DEC     BC
0CAA 78        LD      A,B
0CAB B1        OR      C
0CAC 20F8      JR      NZ,&0CA6         ; (-&08)
0CAE C9        RET     

0CAF 1E00      LD      E,&00
0CB1 18F3      JR      &0CA6            ; (-&0d)
0CB3 2145FB    LD      HL,&FB45
0CB6 CB56      BIT     2,(HL)
0CB8 2043      JR      NZ,&0CFD         ; (+&43)
0CBA 010038    LD      BC,&3800
0CBD EF        RST     &28
0CBE F5        PUSH    AF
0CBF C620      ADD     A,&20
0CC1 CDB00D    CALL    &0DB0
0CC4 F1        POP     AF
0CC5 2600      LD      H,&00
0CC7 6F        LD      L,A
0CC8 29        ADD     HL,HL
0CC9 29        ADD     HL,HL
0CCA 29        ADD     HL,HL
0CCB 44        LD      B,H
0CCC 4D        LD      C,L
0CCD C5        PUSH    BC
0CCE CD5D0D    CALL    &0D5D
0CD1 E7        RST     &20
0CD2 010001    LD      BC,&0100
0CD5 CDAF0C    CALL    &0CAF
0CD8 C1        POP     BC
0CD9 CBE8      SET     5,B
0CDB CD5D0D    CALL    &0D5D
0CDE E7        RST     &20
0CDF 010001    LD      BC,&0100
0CE2 CDA20C    CALL    &0CA2
0CE5 2A4FFB    LD      HL,(&FB4F)
0CE8 2600      LD      H,&00
0CEA E5        PUSH    HL
0CEB 11003C    LD      DE,&3C00
0CEE 19        ADD     HL,DE
0CEF EB        EX      DE,HL
0CF0 44        LD      B,H
0CF1 4D        LD      C,L
0CF2 2A4CFB    LD      HL,(&FB4C)
0CF5 CD680D    CALL    &0D68
0CF8 C1        POP     BC
0CF9 1E20      LD      E,&20
0CFB 18A9      JR      &0CA6            ; (-&57)
0CFD 2A4CFB    LD      HL,(&FB4C)
0D00 015000    LD      BC,&0050
0D03 09        ADD     HL,BC
0D04 7C        LD      A,H
0D05 E607      AND     &07
0D07 67        LD      H,A
0D08 CD300D    CALL    &0D30
0D0B 010017    LD      BC,&1700
0D0E CD170C    CALL    &0C17
0D11 1E50      LD      E,&50
0D13 C5        PUSH    BC
0D14 CDE90B    CALL    &0BE9
0D17 57        LD      D,A
0D18 3E20      LD      A,&20
0D1A CDD90B    CALL    &0BD9
0D1D 3E50      LD      A,&50
0D1F 80        ADD     A,B
0D20 47        LD      B,A
0D21 DCD10B    CALL    C,&0BD1
0D24 7A        LD      A,D
0D25 CDD90B    CALL    &0BD9
0D28 C1        POP     BC
0D29 CDCF0B    CALL    &0BCF
0D2C 1D        DEC     E
0D2D 20E4      JR      NZ,&0D13         ; (-&1c)
0D2F C9        RET     

0D30 3E0C      LD      A,&0C
0D32 D348      OUT     (&48),A
0D34 0E49      LD      C,&49
0D36 ED61      OUT     (C),H
0D38 3C        INC     A
0D39 D348      OUT     (&48),A
0D3B ED69      OUT     (C),L
0D3D 224CFB    LD      (&FB4C),HL
0D40 C9        RET     

0D41 2145FB    LD      HL,&FB45
0D44 CB56      BIT     2,(HL)
0D46 C8        RET     Z

0D47 010018    LD      BC,&1800
0D4A D5        PUSH    DE
0D4B CD170C    CALL    &0C17
0D4E D1        POP     DE
0D4F 2650      LD      H,&50
0D51 1A        LD      A,(DE)
0D52 CDD90B    CALL    &0BD9
0D55 CDCF0B    CALL    &0BCF
0D58 13        INC     DE
0D59 25        DEC     H
0D5A 20F5      JR      NZ,&0D51         ; (-&0b)
0D5C C9        RET     

0D5D CD600D    CALL    &0D60
0D60 59        LD      E,C
0D61 78        LD      A,B
0D62 C608      ADD     A,&08
0D64 57        LD      D,A
0D65 210001    LD      HL,&0100
0D68 D5        PUSH    DE
0D69 C5        PUSH    BC
0D6A 42        LD      B,D
0D6B 4B        LD      C,E
0D6C 112000    LD      DE,&0020
0D6F B7        OR      A
0D70 ED52      SBC     HL,DE
0D72 E5        PUSH    HL
0D73 19        ADD     HL,DE
0D74 3001      JR      NC,&0D77         ; (+&01)
0D76 5D        LD      E,L
0D77 08        EX      AF,AF'
0D78 53        LD      D,E
0D79 D5        PUSH    DE
0D7A CD900D    CALL    &0D90
0D7D D1        POP     DE
0D7E E1        POP     HL
0D7F E3        EX      (SP),HL
0D80 C5        PUSH    BC
0D81 E3        EX      (SP),HL
0D82 C1        POP     BC
0D83 E5        PUSH    HL
0D84 CDA10D    CALL    &0DA1
0D87 D1        POP     DE
0D88 E1        POP     HL
0D89 08        EX      AF,AF'
0D8A 2802      JR      Z,&0D8E          ; (+&02)
0D8C 30DB      JR      NC,&0D69         ; (-&25)
0D8E C1        POP     BC
0D8F C9        RET     

0D90 CD8D0C    CALL    &0C8D
0D93 E5        PUSH    HL
0D94 21E0FB    LD      HL,&FBE0
0D97 DB08      IN      A,(&08)
0D99 77        LD      (HL),A
0D9A 23        INC     HL
0D9B 03        INC     BC
0D9C 15        DEC     D
0D9D 20F8      JR      NZ,&0D97         ; (-&08)
0D9F E1        POP     HL
0DA0 C9        RET     

0DA1 E7        RST     &20
0DA2 E5        PUSH    HL
0DA3 21E0FB    LD      HL,&FBE0
0DA6 7E        LD      A,(HL)
0DA7 D308      OUT     (&08),A
0DA9 23        INC     HL
0DAA 03        INC     BC
0DAB 15        DEC     D
0DAC 20F8      JR      NZ,&0DA6         ; (-&08)
0DAE E1        POP     HL
0DAF C9        RET     

0DB0 010038    LD      BC,&3800
0DB3 E7        RST     &20
0DB4 F5        PUSH    AF
0DB5 07        RLCA    
0DB6 07        RLCA    
0DB7 07        RLCA    
0DB8 324EFB    LD      (&FB4E),A
0DBB F1        POP     AF
0DBC 0603      LD      B,&03
0DBE D308      OUT     (&08),A
0DC0 3C        INC     A
0DC1 0C        INC     C
0DC2 20FA      JR      NZ,&0DBE         ; (-&06)
0DC4 10F8      DJNZ    &0DBE            ; (-&08)
0DC6 C9        RET     

0DC7 CF        RST     &08
0DC8 BD        CP      L
0DC9 01003B    LD      BC,&3B00
0DCC E7        RST     &20
0DCD 018000    LD      BC,&0080
0DD0 CDAF0C    CALL    &0CAF
0DD3 010018    LD      BC,&1800
0DD6 E7        RST     &20
0DD7 0601      LD      B,&01
0DD9 CDAF0C    CALL    &0CAF
0DDC 216D11    LD      HL,&116D
0DDF 3E07      LD      A,&07
0DE1 010800    LD      BC,&0008
0DE4 EDA3      OUTI    
0DE6 F5        PUSH    AF
0DE7 F1        POP     AF
0DE8 20FA      JR      NZ,&0DE4         ; (-&06)
0DEA 3D        DEC     A
0DEB 20F7      JR      NZ,&0DE4         ; (-&09)
0DED 210000    LD      HL,&0000
0DF0 2230FB    LD      (&FB30),HL
0DF3 2232FB    LD      (&FB32),HL
0DF6 2234FB    LD      (&FB34),HL
0DF9 21D324    LD      HL,&24D3
0DFC 223800    LD      (&0038),HL
0DFF 3E01      LD      A,&01
0E01 D320      OUT     (&20),A
0E03 D325      OUT     (&25),A
0E05 D321      OUT     (&21),A
0E07 3E0E      LD      A,&0E
0E09 D7        RST     &10
0E0A C9        RET     

0E0B CD310E    CALL    &0E31
0E0E 20F7      JR      NZ,&0E07         ; (-&09)
0E10 DB4C      IN      A,(&4C)
0E12 CB57      BIT     2,A
0E14 21400E    LD      HL,&0E40
0E17 2003      JR      NZ,&0E1C         ; (+&03)
0E19 21500E    LD      HL,&0E50
0E1C 014910    LD      BC,&1049
0E1F AF        XOR     A
0E20 D348      OUT     (&48),A
0E22 3C        INC     A
0E23 EDA3      OUTI    
0E25 20F9      JR      NZ,&0E20         ; (-&07)
0E27 3E10      LD      A,&10
0E29 D7        RST     &10
0E2A DB4C      IN      A,(&4C)
0E2C CB4F      BIT     1,A
0E2E 28D7      JR      Z,&0E07          ; (-&29)
0E30 C9        RET     

0E31 C5        PUSH    BC
0E32 014000    LD      BC,&0040
0E35 ED78      IN      A,(C)
0E37 2F        CPL     
0E38 ED79      OUT     (C),A
0E3A 57        LD      D,A
0E3B ED78      IN      A,(C)
0E3D BA        CP      D
0E3E C1        POP     BC
0E3F C9        RET     

0E40 7E        LD      A,(HL)
0E41 50        LD      D,B
0E42 61        LD      H,C
0E43 381A      JR      C,&0E5F          ; (+&1a)
0E45 13        INC     DE
0E46 19        ADD     HL,DE
0E47 1A        LD      A,(DE)
0E48 00        NOP     
0E49 08        EX      AF,AF'
0E4A 2008      JR      NZ,&0E54         ; (+&08)
0E4C 00        NOP     
0E4D 00        NOP     
0E4E 00        NOP     
0E4F 00        NOP     
0E50 7F        LD      A,A
0E51 50        LD      D,B
0E52 62        LD      H,D
0E53 381E      JR      C,&0E73          ; (+&1e)
0E55 02        LD      (BC),A
0E56 19        ADD     HL,DE
0E57 1B        DEC     DE
0E58 00        NOP     
0E59 09        ADD     HL,BC
0E5A 2009      JR      NZ,&0E65         ; (+&09)
0E5C 00        NOP     
0E5D 00        NOP     
0E5E 00        NOP     
0E5F 00        NOP     
0E60 78        LD      A,B
0E61 210000    LD      HL,&0000
0E64 0610      LD      B,&10
0E66 1F        RRA     
0E67 CB19      RR      C
0E69 3001      JR      NC,&0E6C         ; (+&01)
0E6B 19        ADD     HL,DE
0E6C CB1C      RR      H
0E6E CB1D      RR      L
0E70 1F        RRA     
0E71 CB19      RR      C
0E73 10F4      DJNZ    &0E69            ; (-&0c)
0E75 EB        EX      DE,HL
0E76 67        LD      H,A
0E77 69        LD      L,C
0E78 7A        LD      A,D
0E79 B3        OR      E
0E7A C9        RET     

0E7B DF        RST     &18
0E7C 0C        INC     C
0E7D 2020      JR      NZ,&0E9F         ; (+&20)
0E7F 2020      JR      NZ,&0EA1         ; (+&20)
0E81 2020      JR      NZ,&0EA3         ; (+&20)
0E83 2020      JR      NZ,&0EA5         ; (+&20)
0E85 2020      JR      NZ,&0EA7         ; (+&20)
0E87 202A      JR      NZ,&0EB3         ; (+&2a)
0E89 2A2A20    LD      HL,(&202A)
0E8C 2045      JR      NZ,&0ED3         ; (+&45)
0E8E 49        LD      C,C
0E8F 4E        LD      C,(HL)
0E90 53        LD      D,E
0E91 54        LD      D,H
0E92 45        LD      B,L
0E93 49        LD      C,C
0E94 4E        LD      C,(HL)
0E95 2020      JR      NZ,&0EB7         ; (+&20)
0E97 2A2A2A    LD      HL,(&2A2A)
0E9A 0D        DEC     C
0E9B 8A        ADC     A,D
0E9C C9        RET     

0E9D 7C        LD      A,H
0E9E CF        RST     &08
0E9F AB        XOR     E
0EA0 7D        LD      A,L
0EA1 CF        RST     &08
0EA2 AB        XOR     E
0EA3 1873      JR      &0F18            ; (+&73)
0EA5 F5        PUSH    AF
0EA6 0F        RRCA    
0EA7 0F        RRCA    
0EA8 0F        RRCA    
0EA9 0F        RRCA    
0EAA CDAE0E    CALL    &0EAE
0EAD F1        POP     AF
0EAE E60F      AND     &0F
0EB0 C690      ADD     A,&90
0EB2 27        DAA     
0EB3 CE40      ADC     A,&40
0EB5 27        DAA     
0EB6 D7        RST     &10
0EB7 B7        OR      A
0EB8 C9        RET     

0EB9 EB        EX      DE,HL
0EBA CF        RST     &08
0EBB B1        OR      C
0EBC EB        EX      DE,HL
0EBD 13        INC     DE
0EBE D630      SUB     &30
0EC0 FE0A      CP      &0A
0EC2 3F        CCF     
0EC3 D0        RET     NC

0EC4 C630      ADD     A,&30
0EC6 E65F      AND     &5F
0EC8 D637      SUB     &37
0ECA FE10      CP      &10
0ECC 3F        CCF     
0ECD D0        RET     NC

0ECE 3E00      LD      A,&00
0ED0 C9        RET     

0ED1 C5        PUSH    BC
0ED2 CDB90E    CALL    &0EB9
0ED5 380B      JR      C,&0EE2          ; (+&0b)
0ED7 07        RLCA    
0ED8 07        RLCA    
0ED9 07        RLCA    
0EDA 07        RLCA    
0EDB 4F        LD      C,A
0EDC CDB90E    CALL    &0EB9
0EDF 3801      JR      C,&0EE2          ; (+&01)
0EE1 B1        OR      C
0EE2 C1        POP     BC
0EE3 C9        RET     

0EE4 3E04      LD      A,&04
0EE6 C5        PUSH    BC
0EE7 47        LD      B,A
0EE8 AF        XOR     A
0EE9 67        LD      H,A
0EEA 6F        LD      L,A
0EEB CDB90E    CALL    &0EB9
0EEE 3809      JR      C,&0EF9          ; (+&09)
0EF0 29        ADD     HL,HL
0EF1 29        ADD     HL,HL
0EF2 29        ADD     HL,HL
0EF3 29        ADD     HL,HL
0EF4 B5        OR      L
0EF5 6F        LD      L,A
0EF6 10F3      DJNZ    &0EEB            ; (-&0d)
0EF8 13        INC     DE
0EF9 1B        DEC     DE
0EFA C1        POP     BC
0EFB C9        RET     

0EFC 13        INC     DE
0EFD 1A        LD      A,(DE)
0EFE FE20      CP      &20
0F00 28FA      JR      Z,&0EFC          ; (-&06)
0F02 FE2C      CP      &2C
0F04 28F6      JR      Z,&0EFC          ; (-&0a)
0F06 B7        OR      A
0F07 C9        RET     

0F08 EB        EX      DE,HL
0F09 5E        LD      E,(HL)
0F0A 23        INC     HL
0F0B 56        LD      D,(HL)
0F0C 23        INC     HL
0F0D EB        EX      DE,HL
0F0E C9        RET     

0F0F 3A4AFB    LD      A,(&FB4A)
0F12 B7        OR      A
0F13 C8        RET     Z

0F14 DF        RST     &18
0F15 0D        DEC     C
0F16 8A        ADC     A,D
0F17 C9        RET     

0F18 3E20      LD      A,&20
0F1A E5        PUSH    HL
0F1B D5        PUSH    DE
0F1C C5        PUSH    BC
0F1D F5        PUSH    AF
0F1E CD9608    CALL    &0896
0F21 F1        POP     AF
0F22 C1        POP     BC
0F23 D1        POP     DE
0F24 E1        POP     HL
0F25 C9        RET     

0F26 E5        PUSH    HL
0F27 2A7BFB    LD      HL,(&FB7B)
0F2A CF        RST     &08
0F2B B1        OR      C
0F2C 5F        LD      E,A
0F2D 23        INC     HL
0F2E CF        RST     &08
0F2F B1        OR      C
0F30 57        LD      D,A
0F31 EB        EX      DE,HL
0F32 CF        RST     &08
0F33 B1        OR      C
0F34 CD400F    CALL    &0F40
0F37 28F9      JR      Z,&0F32          ; (-&07)
0F39 EB        EX      DE,HL
0F3A 72        LD      (HL),D
0F3B 2B        DEC     HL
0F3C 73        LD      (HL),E
0F3D E1        POP     HL
0F3E C9        RET     

0F3F 7E        LD      A,(HL)
0F40 CB7F      BIT     7,A
0F42 CBBF      RES     7,A
0F44 D7        RST     &10
0F45 23        INC     HL
0F46 C9        RET     

0F47 79        LD      A,C
0F48 F5        PUSH    AF
0F49 DB11      IN      A,(&11)
0F4B 1F        RRA     
0F4C 30FB      JR      NC,&0F49         ; (-&05)
0F4E F1        POP     AF
0F4F D310      OUT     (&10),A
0F51 C9        RET     

0F52 DB11      IN      A,(&11)
0F54 1F        RRA     
0F55 1F        RRA     
0F56 30FA      JR      NC,&0F52         ; (-&06)
0F58 DB10      IN      A,(&10)
0F5A C9        RET     

0F5B 79        LD      A,C
0F5C E5        PUSH    HL
0F5D 2192FB    LD      HL,&FB92
0F60 CB46      BIT     0,(HL)
0F62 20FC      JR      NZ,&0F60         ; (-&04)
0F64 D330      OUT     (&30),A
0F66 CBC6      SET     0,(HL)
0F68 E1        POP     HL
0F69 C9        RET     

0F6A 79        LD      A,C
0F6B D302      OUT     (&02),A
0F6D 78        LD      A,B
0F6E D303      OUT     (&03),A
0F70 C9        RET     

0F71 01008E    LD      BC,&8E00
0F74 CF        RST     &08
0F75 B4        OR      H
0F76 010100    LD      BC,&0001
0F79 CF        RST     &08
0F7A B4        OR      H
0F7B 01080A    LD      BC,&0A08
0F7E CF        RST     &08
0F7F B4        OR      H
0F80 01077E    LD      BC,&7E07
0F83 CF        RST     &08
0F84 B4        OR      H
0F85 012E3A    LD      BC,&3A2E
0F88 0B        DEC     BC
0F89 78        LD      A,B
0F8A B1        OR      C
0F8B 20FB      JR      NZ,&0F88         ; (-&05)
0F8D 010800    LD      BC,&0008
0F90 CF        RST     &08
0F91 B4        OR      H
0F92 01077F    LD      BC,&7F07
0F95 CF        RST     &08
0F96 B4        OR      H
0F97 1EFF      LD      E,&FF
0F99 3E0E      LD      A,&0E
0F9B D302      OUT     (&02),A
0F9D 7B        LD      A,E
0F9E 2F        CPL     
0F9F D303      OUT     (&03),A
0FA1 C9        RET     

0FA2 CD990F    CALL    &0F99
0FA5 3E0F      LD      A,&0F
0FA7 D302      OUT     (&02),A
0FA9 DB02      IN      A,(&02)
0FAB 2F        CPL     
0FAC B7        OR      A
0FAD C9        RET     

0FAE 0E2A      LD      C,&2A
0FB0 3E3F      LD      A,&3F
0FB2 067D      LD      B,&7D
0FB4 CDBC0F    CALL    &0FBC
0FB7 0C        INC     C
0FB8 3EDF      LD      A,&DF
0FBA 067D      LD      B,&7D
0FBC ED79      OUT     (C),A
0FBE 00        NOP     
0FBF ED41      OUT     (C),B
0FC1 C9        RET     

0FC2 2279FB    LD      (&FB79),HL
0FC5 E1        POP     HL
0FC6 F5        PUSH    AF
0FC7 7E        LD      A,(HL)
0FC8 23        INC     HL
0FC9 E3        EX      (SP),HL
0FCA E5        PUSH    HL
0FCB CB7F      BIT     7,A
0FCD CBBF      RES     7,A
0FCF 2813      JR      Z,&0FE4          ; (+&13)
0FD1 2A3AFB    LD      HL,(&FB3A)
0FD4 87        ADD     A,A
0FD5 85        ADD     A,L
0FD6 6F        LD      L,A
0FD7 3001      JR      NC,&0FDA         ; (+&01)
0FD9 24        INC     H
0FDA 7E        LD      A,(HL)
0FDB 23        INC     HL
0FDC 66        LD      H,(HL)
0FDD 6F        LD      L,A
0FDE F1        POP     AF
0FDF E5        PUSH    HL
0FE0 2A79FB    LD      HL,(&FB79)
0FE3 C9        RET     

0FE4 2A3CFB    LD      HL,(&FB3C)
0FE7 E9        JP      (HL)
0FE8 02        LD      (BC),A
0FE9 C0        RET     NZ

0FEA 0EFF      LD      C,&FF
0FEC 03        INC     BC
0FED 76        HALT    
0FEE 03        INC     BC
0FEF F44EFC    CALL    P,&FC4E
0FF2 4E        LD      C,(HL)
0FF3 FC4EFC    CALL    M,&FC4E
0FF6 47        LD      B,A
0FF7 FC4EFC    CALL    M,&FC4E
0FFA 4E        LD      C,(HL)
0FFB FC4EFC    CALL    M,&FC4E
0FFE 4E        LD      C,(HL)
0FFF FC84FC    CALL    M,&FC84
1002 4E        LD      C,(HL)
1003 FCD324    CALL    M,&24D3
1006 7E        LD      A,(HL)
1007 D324      OUT     (&24),A
1009 C9        RET     

100A D324      OUT     (&24),A
100C EDB0      LDIR    
100E D324      OUT     (&24),A
1010 C9        RET     

1011 D324      OUT     (&24),A
1013 EDB8      LDDR    
1015 D324      OUT     (&24),A
1017 C9        RET     

1018 D324      OUT     (&24),A
101A CDCA0F    CALL    &0FCA
101D D324      OUT     (&24),A
101F C9        RET     

1020 00        NOP     
1021 00        NOP     
1022 00        NOP     
1023 00        NOP     
1024 00        NOP     
1025 00        NOP     
1026 02        LD      (BC),A
1027 C0        RET     NZ

1028 F0        RET     P

1029 F0        RET     P

102A 82        ADD     A,D
102B 02        LD      (BC),A
102C 7D        LD      A,L
102D 00        NOP     
102E 80        ADD     A,B
102F 7F        LD      A,A
1030 3E20      LD      A,&20
1032 A0        AND     B
1033 1002      DJNZ    &1037            ; (+&02)
1035 01CF9A    LD      BC,&9ACF
1038 FF        RST     &38
1039 00        NOP     
103A 00        NOP     
103B FF        RST     &38
103C FF        RST     &38
103D FF        RST     &38
103E C322FC    JP      &FC22
1041 3A3800    LD      A,(&0038)
1044 B7        OR      A
1045 C8        RET     Z

1046 180F      JR      &1057            ; (+&0f)
1048 3A3800    LD      A,(&0038)
104B B7        OR      A
104C C0        RET     NZ

104D 1808      JR      &1057            ; (+&08)
104F C5        PUSH    BC
1050 4F        LD      C,A
1051 3A3800    LD      A,(&0038)
1054 B9        CP      C
1055 C1        POP     BC
1056 C8        RET     Z

1057 D324      OUT     (&24),A
1059 C9        RET     

105A 2279FB    LD      (&FB79),HL
105D E1        POP     HL
105E F5        PUSH    AF
105F 7E        LD      A,(HL)
1060 23        INC     HL
1061 E3        EX      (SP),HL
1062 ED737BFB  LD      (&FB7B),SP
1066 E5        PUSH    HL
1067 B7        OR      A
1068 2100FB    LD      HL,&FB00
106B ED72      SBC     HL,SP
106D E1        POP     HL
106E 3803      JR      C,&1073          ; (+&03)
1070 31FFFC    LD      SP,&FCFF
1073 D324      OUT     (&24),A
1075 CDCA0F    CALL    &0FCA
1078 D324      OUT     (&24),A
107A ED7B7BFB  LD      SP,(&FB7B)
107E C9        RET     

107F E5        PUSH    HL
1080 F5        PUSH    AF
1081 CD51FC    CALL    &FC51
1084 F1        POP     AF
1085 E1        POP     HL
1086 FB        EI      
1087 ED4D      RETI    

1089 2191FB    LD      HL,&FB91
108C 3E39      LD      A,&39
108E CD7DFC    CALL    &FC7D
1091 D0        RET     NC

1092 3E35      LD      A,&35
1094 CD7DFC    CALL    &FC7D
1097 D0        RET     NC

1098 3E39      LD      A,&39
109A CD7DFC    CALL    &FC7D
109D D0        RET     NC

109E 3E35      LD      A,&35
10A0 CD7DFC    CALL    &FC7D
10A3 D0        RET     NC

10A4 2B        DEC     HL
10A5 3E32      LD      A,&32
10A7 BE        CP      (HL)
10A8 23        INC     HL
10A9 3E39      LD      A,&39
10AB 2002      JR      NZ,&10AF         ; (+&02)
10AD 3E33      LD      A,&33
10AF CD7DFC    CALL    &FC7D
10B2 D0        RET     NC

10B3 3E32      LD      A,&32
10B5 34        INC     (HL)
10B6 BE        CP      (HL)
10B7 D0        RET     NC

10B8 3630      LD      (HL),&30
10BA 2B        DEC     HL
10BB C9        RET     

10BC E5        PUSH    HL
10BD 2192FB    LD      HL,&FB92
10C0 CB86      RES     0,(HL)
10C2 E1        POP     HL
10C3 18C1      JR      &1086            ; (-&3f)
10C5 69        LD      L,C
10C6 6F        LD      L,A
10C7 70        LD      (HL),B
10C8 5B        LD      E,E
10C9 5F        LD      E,A
10CA 0A        LD      A,(BC)
10CB 7C        LD      A,H
10CC 3049      JR      NC,&1117         ; (+&49)
10CE 4F        LD      C,A
10CF 50        LD      D,B
10D0 7B        LD      A,E
10D1 60        LD      H,B
10D2 0B        DEC     BC
10D3 5C        LD      E,H
10D4 40        LD      B,B
10D5 09        ADD     HL,BC
10D6 0F        RRCA    
10D7 101B      DJNZ    &10F4            ; (+&1b)
10D9 1F        RRA     
10DA 0A        LD      A,(BC)
10DB 1C        INC     E
10DC 306B      JR      NC,&1149         ; (+&6b)
10DE 6C        LD      L,H
10DF 3B        DEC     SP
10E0 3A5D04    LD      A,(&045D)
10E3 39        ADD     HL,SP
10E4 85        ADD     A,L
10E5 4B        LD      C,E
10E6 4C        LD      C,H
10E7 2B        DEC     HL
10E8 2A7D08    LD      HL,(&087D)
10EB 29        ADD     HL,HL
10EC 85        ADD     A,L
10ED 0B        DEC     BC
10EE 0C        INC     C
10EF 3B        DEC     SP
10F0 3A1D04    LD      A,(&041D)
10F3 39        ADD     HL,SP
10F4 85        ADD     A,L
10F5 2C        INC     L
10F6 2E2F      LD      L,&2F
10F8 3819      JR      C,&1113          ; (+&19)
10FA 3D        DEC     A
10FB 5E        LD      E,(HL)
10FC 84        ADD     A,H
10FD 3C        INC     A
10FE 3E3F      LD      A,&3F
1100 281A      JR      Z,&111C          ; (+&1a)
1102 2D        DEC     L
1103 7E        LD      A,(HL)
1104 84        ADD     A,H
1105 2C        INC     L
1106 2E2F      LD      L,&2F
1108 3806      JR      C,&1110          ; (+&06)
110A 3D        DEC     A
110B 1E84      LD      E,&84
110D 37        SCF     
110E 3635      LD      (HL),&35
1110 34        INC     (HL)
1111 33        INC     SP
1112 323183    LD      (&8331),A
1115 27        DAA     
1116 2625      LD      H,&25
1118 24        INC     H
1119 23        INC     HL
111A 222183    LD      (&8321),HL
111D 37        SCF     
111E 3635      LD      (HL),&35
1120 34        INC     (HL)
1121 33        INC     SP
1122 323183    LD      (&8331),A
1125 75        LD      (HL),L
1126 79        LD      A,C
1127 74        LD      (HL),H
1128 72        LD      (HL),D
1129 65        LD      H,L
112A 77        LD      (HL),A
112B 71        LD      (HL),C
112C 82        ADD     A,D
112D 55        LD      D,L
112E 59        LD      E,C
112F 54        LD      D,H
1130 52        LD      D,D
1131 45        LD      B,L
1132 57        LD      D,A
1133 51        LD      D,C
1134 82        ADD     A,D
1135 15        DEC     D
1136 19        ADD     HL,DE
1137 14        INC     D
1138 12        LD      (DE),A
1139 05        DEC     B
113A 17        RLA     
113B 11826A    LD      DE,&6A82
113E 68        LD      L,B
113F 67        LD      H,A
1140 66        LD      H,(HL)
1141 64        LD      H,H
1142 73        LD      (HL),E
1143 61        LD      H,C
1144 81        ADD     A,C
1145 4A        LD      C,D
1146 48        LD      C,B
1147 47        LD      B,A
1148 46        LD      B,(HL)
1149 44        LD      B,H
114A 53        LD      D,E
114B 41        LD      B,C
114C 81        ADD     A,C
114D 0A        LD      A,(BC)
114E 08        EX      AF,AF'
114F 07        RLCA    
1150 0604      LD      B,&04
1152 13        INC     DE
1153 01816D    LD      BC,&6D81
1156 6E        LD      L,(HL)
1157 62        LD      H,D
1158 76        HALT    
1159 63        LD      H,E
115A 78        LD      A,B
115B 7A        LD      A,D
115C 86        ADD     A,(HL)
115D 4D        LD      C,L
115E 4E        LD      C,(HL)
115F 42        LD      B,D
1160 56        LD      D,(HL)
1161 43        LD      B,E
1162 58        LD      E,B
1163 5A        LD      E,D
1164 86        ADD     A,(HL)
1165 0D        DEC     C
1166 0E02      LD      C,&02
1168 1603      LD      D,&03
116A 181A      JR      &1186            ; (+&1a)
116C 86        ADD     A,(HL)
116D 00        NOP     
116E 00        NOP     
116F 00        NOP     
1170 00        NOP     
1171 00        NOP     
1172 00        NOP     
1173 00        NOP     
1174 00        NOP     
1175 00        NOP     
1176 1010      DJNZ    &1188            ; (+&10)
1178 1010      DJNZ    &118A            ; (+&10)
117A 00        NOP     
117B 1000      DJNZ    &117D            ; (+&00)
117D 00        NOP     
117E 2828      JR      Z,&11A8          ; (+&28)
1180 2800      JR      Z,&1182          ; (+&00)
1182 00        NOP     
1183 00        NOP     
1184 00        NOP     
1185 00        NOP     
1186 287C      JR      Z,&1204          ; (+&7c)
1188 2828      JR      Z,&11B2          ; (+&28)
118A 7C        LD      A,H
118B 2800      JR      Z,&118D          ; (+&00)
118D 00        NOP     
118E 103C      DJNZ    &11CC            ; (+&3c)
1190 50        LD      D,B
1191 3814      JR      C,&11A7          ; (+&14)
1193 78        LD      A,B
1194 1000      DJNZ    &1196            ; (+&00)
1196 60        LD      H,B
1197 64        LD      H,H
1198 08        EX      AF,AF'
1199 102C      DJNZ    &11C7            ; (+&2c)
119B 4C        LD      C,H
119C 00        NOP     
119D 00        NOP     
119E 2050      JR      NZ,&11F0         ; (+&50)
11A0 2054      JR      NZ,&11F6         ; (+&54)
11A2 48        LD      C,B
11A3 34        INC     (HL)
11A4 00        NOP     
11A5 00        NOP     
11A6 1010      DJNZ    &11B8            ; (+&10)
11A8 1000      DJNZ    &11AA            ; (+&00)
11AA 00        NOP     
11AB 00        NOP     
11AC 00        NOP     
11AD 00        NOP     
11AE 08        EX      AF,AF'
11AF 1010      DJNZ    &11C1            ; (+&10)
11B1 1010      DJNZ    &11C3            ; (+&10)
11B3 08        EX      AF,AF'
11B4 00        NOP     
11B5 00        NOP     
11B6 2010      JR      NZ,&11C8         ; (+&10)
11B8 1010      DJNZ    &11CA            ; (+&10)
11BA 1020      DJNZ    &11DC            ; (+&20)
11BC 00        NOP     
11BD 00        NOP     
11BE 1054      DJNZ    &1214            ; (+&54)
11C0 3810      JR      C,&11D2          ; (+&10)
11C2 3854      JR      C,&1218          ; (+&54)
11C4 1000      DJNZ    &11C6            ; (+&00)
11C6 00        NOP     
11C7 1010      DJNZ    &11D9            ; (+&10)
11C9 7C        LD      A,H
11CA 1010      DJNZ    &11DC            ; (+&10)
11CC 00        NOP     
11CD 00        NOP     
11CE 00        NOP     
11CF 00        NOP     
11D0 00        NOP     
11D1 00        NOP     
11D2 1010      DJNZ    &11E4            ; (+&10)
11D4 2000      JR      NZ,&11D6         ; (+&00)
11D6 00        NOP     
11D7 00        NOP     
11D8 00        NOP     
11D9 3800      JR      C,&11DB          ; (+&00)
11DB 00        NOP     
11DC 00        NOP     
11DD 00        NOP     
11DE 00        NOP     
11DF 00        NOP     
11E0 00        NOP     
11E1 00        NOP     
11E2 00        NOP     
11E3 1000      DJNZ    &11E5            ; (+&00)
11E5 00        NOP     
11E6 00        NOP     
11E7 04        INC     B
11E8 08        EX      AF,AF'
11E9 1020      DJNZ    &120B            ; (+&20)
11EB 40        LD      B,B
11EC 00        NOP     
11ED 00        NOP     
11EE 1028      DJNZ    &1218            ; (+&28)
11F0 44        LD      B,H
11F1 44        LD      B,H
11F2 2810      JR      Z,&1204          ; (+&10)
11F4 00        NOP     
11F5 00        NOP     
11F6 1030      DJNZ    &1228            ; (+&30)
11F8 1010      DJNZ    &120A            ; (+&10)
11FA 1038      DJNZ    &1234            ; (+&38)
11FC 00        NOP     
11FD 00        NOP     
11FE 3844      JR      C,&1244          ; (+&44)
1200 04        INC     B
1201 3840      JR      C,&1243          ; (+&40)
1203 7C        LD      A,H
1204 00        NOP     
1205 00        NOP     
1206 3844      JR      C,&124C          ; (+&44)
1208 1804      JR      &120E            ; (+&04)
120A 44        LD      B,H
120B 3800      JR      C,&120D          ; (+&00)
120D 00        NOP     
120E 1828      JR      &1238            ; (+&28)
1210 48        LD      C,B
1211 7C        LD      A,H
1212 08        EX      AF,AF'
1213 08        EX      AF,AF'
1214 00        NOP     
1215 00        NOP     
1216 7C        LD      A,H
1217 40        LD      B,B
1218 78        LD      A,B
1219 04        INC     B
121A 44        LD      B,H
121B 3800      JR      C,&121D          ; (+&00)
121D 00        NOP     
121E 3840      JR      C,&1260          ; (+&40)
1220 78        LD      A,B
1221 44        LD      B,H
1222 44        LD      B,H
1223 3800      JR      C,&1225          ; (+&00)
1225 00        NOP     
1226 7C        LD      A,H
1227 04        INC     B
1228 08        EX      AF,AF'
1229 1020      DJNZ    &124B            ; (+&20)
122B 2000      JR      NZ,&122D         ; (+&00)
122D 00        NOP     
122E 3844      JR      C,&1274          ; (+&44)
1230 3844      JR      C,&1276          ; (+&44)
1232 44        LD      B,H
1233 3800      JR      C,&1235          ; (+&00)
1235 00        NOP     
1236 3844      JR      C,&127C          ; (+&44)
1238 3804      JR      C,&123E          ; (+&04)
123A 08        EX      AF,AF'
123B 1000      DJNZ    &123D            ; (+&00)
123D 00        NOP     
123E 00        NOP     
123F 00        NOP     
1240 1000      DJNZ    &1242            ; (+&00)
1242 1000      DJNZ    &1244            ; (+&00)
1244 00        NOP     
1245 00        NOP     
1246 00        NOP     
1247 00        NOP     
1248 1000      DJNZ    &124A            ; (+&00)
124A 1010      DJNZ    &125C            ; (+&10)
124C 2000      JR      NZ,&124E         ; (+&00)
124E 00        NOP     
124F 1020      DJNZ    &1271            ; (+&20)
1251 40        LD      B,B
1252 2010      JR      NZ,&1264         ; (+&10)
1254 00        NOP     
1255 00        NOP     
1256 00        NOP     
1257 00        NOP     
1258 7C        LD      A,H
1259 00        NOP     
125A 7C        LD      A,H
125B 00        NOP     
125C 00        NOP     
125D 00        NOP     
125E 00        NOP     
125F 1008      DJNZ    &1269            ; (+&08)
1261 04        INC     B
1262 08        EX      AF,AF'
1263 1000      DJNZ    &1265            ; (+&00)
1265 00        NOP     
1266 3844      JR      C,&12AC          ; (+&44)
1268 08        EX      AF,AF'
1269 1010      DJNZ    &127B            ; (+&10)
126B 00        NOP     
126C 1000      DJNZ    &126E            ; (+&00)
126E 3844      JR      C,&12B4          ; (+&44)
1270 54        LD      D,H
1271 5C        LD      E,H
1272 40        LD      B,B
1273 3C        INC     A
1274 00        NOP     
1275 00        NOP     
1276 1028      DJNZ    &12A0            ; (+&28)
1278 44        LD      B,H
1279 7C        LD      A,H
127A 44        LD      B,H
127B 44        LD      B,H
127C 00        NOP     
127D 00        NOP     
127E 78        LD      A,B
127F 44        LD      B,H
1280 78        LD      A,B
1281 44        LD      B,H
1282 44        LD      B,H
1283 78        LD      A,B
1284 00        NOP     
1285 00        NOP     
1286 3844      JR      C,&12CC          ; (+&44)
1288 40        LD      B,B
1289 40        LD      B,B
128A 44        LD      B,H
128B 3800      JR      C,&128D          ; (+&00)
128D 00        NOP     
128E 78        LD      A,B
128F 44        LD      B,H
1290 44        LD      B,H
1291 44        LD      B,H
1292 44        LD      B,H
1293 78        LD      A,B
1294 00        NOP     
1295 00        NOP     
1296 7C        LD      A,H
1297 40        LD      B,B
1298 78        LD      A,B
1299 40        LD      B,B
129A 40        LD      B,B
129B 7C        LD      A,H
129C 00        NOP     
129D 00        NOP     
129E 7C        LD      A,H
129F 40        LD      B,B
12A0 78        LD      A,B
12A1 40        LD      B,B
12A2 40        LD      B,B
12A3 40        LD      B,B
12A4 00        NOP     
12A5 00        NOP     
12A6 3844      JR      C,&12EC          ; (+&44)
12A8 40        LD      B,B
12A9 4C        LD      C,H
12AA 44        LD      B,H
12AB 3800      JR      C,&12AD          ; (+&00)
12AD 00        NOP     
12AE 44        LD      B,H
12AF 44        LD      B,H
12B0 7C        LD      A,H
12B1 44        LD      B,H
12B2 44        LD      B,H
12B3 44        LD      B,H
12B4 00        NOP     
12B5 00        NOP     
12B6 7C        LD      A,H
12B7 1010      DJNZ    &12C9            ; (+&10)
12B9 1010      DJNZ    &12CB            ; (+&10)
12BB 7C        LD      A,H
12BC 00        NOP     
12BD 00        NOP     
12BE 3C        INC     A
12BF 08        EX      AF,AF'
12C0 08        EX      AF,AF'
12C1 08        EX      AF,AF'
12C2 48        LD      C,B
12C3 3000      JR      NC,&12C5         ; (+&00)
12C5 00        NOP     
12C6 44        LD      B,H
12C7 48        LD      C,B
12C8 50        LD      D,B
12C9 70        LD      (HL),B
12CA 48        LD      C,B
12CB 44        LD      B,H
12CC 00        NOP     
12CD 00        NOP     
12CE 40        LD      B,B
12CF 40        LD      B,B
12D0 40        LD      B,B
12D1 40        LD      B,B
12D2 40        LD      B,B
12D3 7C        LD      A,H
12D4 00        NOP     
12D5 00        NOP     
12D6 44        LD      B,H
12D7 6C        LD      L,H
12D8 54        LD      D,H
12D9 54        LD      D,H
12DA 44        LD      B,H
12DB 44        LD      B,H
12DC 00        NOP     
12DD 00        NOP     
12DE 44        LD      B,H
12DF 64        LD      H,H
12E0 54        LD      D,H
12E1 4C        LD      C,H
12E2 44        LD      B,H
12E3 44        LD      B,H
12E4 00        NOP     
12E5 00        NOP     
12E6 3844      JR      C,&132C          ; (+&44)
12E8 44        LD      B,H
12E9 44        LD      B,H
12EA 44        LD      B,H
12EB 3800      JR      C,&12ED          ; (+&00)
12ED 00        NOP     
12EE 78        LD      A,B
12EF 44        LD      B,H
12F0 78        LD      A,B
12F1 40        LD      B,B
12F2 40        LD      B,B
12F3 40        LD      B,B
12F4 00        NOP     
12F5 00        NOP     
12F6 3844      JR      C,&133C          ; (+&44)
12F8 44        LD      B,H
12F9 54        LD      D,H
12FA 48        LD      C,B
12FB 34        INC     (HL)
12FC 00        NOP     
12FD 00        NOP     
12FE 78        LD      A,B
12FF 44        LD      B,H
1300 78        LD      A,B
1301 50        LD      D,B
1302 48        LD      C,B
1303 44        LD      B,H
1304 00        NOP     
1305 00        NOP     
1306 3840      JR      C,&1348          ; (+&40)
1308 3804      JR      C,&130E          ; (+&04)
130A 04        INC     B
130B 3800      JR      C,&130D          ; (+&00)
130D 00        NOP     
130E 7C        LD      A,H
130F 1010      DJNZ    &1321            ; (+&10)
1311 1010      DJNZ    &1323            ; (+&10)
1313 1000      DJNZ    &1315            ; (+&00)
1315 00        NOP     
1316 44        LD      B,H
1317 44        LD      B,H
1318 44        LD      B,H
1319 44        LD      B,H
131A 44        LD      B,H
131B 3800      JR      C,&131D          ; (+&00)
131D 00        NOP     
131E 44        LD      B,H
131F 44        LD      B,H
1320 44        LD      B,H
1321 2828      JR      Z,&134B          ; (+&28)
1323 1000      DJNZ    &1325            ; (+&00)
1325 00        NOP     
1326 44        LD      B,H
1327 44        LD      B,H
1328 54        LD      D,H
1329 54        LD      D,H
132A 54        LD      D,H
132B 2800      JR      Z,&132D          ; (+&00)
132D 00        NOP     
132E 44        LD      B,H
132F 2810      JR      Z,&1341          ; (+&10)
1331 1028      DJNZ    &135B            ; (+&28)
1333 44        LD      B,H
1334 00        NOP     
1335 00        NOP     
1336 44        LD      B,H
1337 44        LD      B,H
1338 2810      JR      Z,&134A          ; (+&10)
133A 1010      DJNZ    &134C            ; (+&10)
133C 00        NOP     
133D 00        NOP     
133E 7C        LD      A,H
133F 08        EX      AF,AF'
1340 1020      DJNZ    &1362            ; (+&20)
1342 40        LD      B,B
1343 7C        LD      A,H
1344 00        NOP     
1345 00        NOP     
1346 00        NOP     
1347 1020      DJNZ    &1369            ; (+&20)
1349 7C        LD      A,H
134A 2010      JR      NZ,&135C         ; (+&10)
134C 00        NOP     
134D 00        NOP     
134E 40        LD      B,B
134F 40        LD      B,B
1350 58        LD      E,B
1351 04        INC     B
1352 181C      JR      &1370            ; (+&1c)
1354 00        NOP     
1355 00        NOP     
1356 00        NOP     
1357 1008      DJNZ    &1361            ; (+&08)
1359 7C        LD      A,H
135A 08        EX      AF,AF'
135B 1000      DJNZ    &135D            ; (+&00)
135D 00        NOP     
135E 1038      DJNZ    &1398            ; (+&38)
1360 54        LD      D,H
1361 1010      DJNZ    &1373            ; (+&10)
1363 1000      DJNZ    &1365            ; (+&00)
1365 00        NOP     
1366 00        NOP     
1367 00        NOP     
1368 00        NOP     
1369 00        NOP     
136A 00        NOP     
136B 00        NOP     
136C 7C        LD      A,H
136D 00        NOP     
136E 1824      JR      &1394            ; (+&24)
1370 2070      JR      NZ,&13E2         ; (+&70)
1372 207C      JR      NZ,&13F0         ; (+&7c)
1374 00        NOP     
1375 00        NOP     
1376 00        NOP     
1377 3008      JR      NC,&1381         ; (+&08)
1379 3848      JR      C,&13C3          ; (+&48)
137B 34        INC     (HL)
137C 00        NOP     
137D 00        NOP     
137E 40        LD      B,B
137F 40        LD      B,B
1380 70        LD      (HL),B
1381 48        LD      C,B
1382 48        LD      C,B
1383 70        LD      (HL),B
1384 00        NOP     
1385 00        NOP     
1386 00        NOP     
1387 3048      JR      NC,&13D1         ; (+&48)
1389 40        LD      B,B
138A 48        LD      C,B
138B 3000      JR      NC,&138D         ; (+&00)
138D 00        NOP     
138E 08        EX      AF,AF'
138F 08        EX      AF,AF'
1390 3848      JR      C,&13DA          ; (+&48)
1392 48        LD      C,B
1393 3800      JR      C,&1395          ; (+&00)
1395 00        NOP     
1396 00        NOP     
1397 3048      JR      NC,&13E1         ; (+&48)
1399 70        LD      (HL),B
139A 40        LD      B,B
139B 3800      JR      C,&139D          ; (+&00)
139D 00        NOP     
139E 1028      DJNZ    &13C8            ; (+&28)
13A0 2078      JR      NZ,&141A         ; (+&78)
13A2 2020      JR      NZ,&13C4         ; (+&20)
13A4 00        NOP     
13A5 00        NOP     
13A6 00        NOP     
13A7 3048      JR      NC,&13F1         ; (+&48)
13A9 3808      JR      C,&13B3          ; (+&08)
13AB 48        LD      C,B
13AC 3000      JR      NC,&13AE         ; (+&00)
13AE 40        LD      B,B
13AF 40        LD      B,B
13B0 70        LD      (HL),B
13B1 48        LD      C,B
13B2 48        LD      C,B
13B3 48        LD      C,B
13B4 00        NOP     
13B5 00        NOP     
13B6 1000      DJNZ    &13B8            ; (+&00)
13B8 3010      JR      NC,&13CA         ; (+&10)
13BA 1038      DJNZ    &13F4            ; (+&38)
13BC 00        NOP     
13BD 00        NOP     
13BE 08        EX      AF,AF'
13BF 00        NOP     
13C0 08        EX      AF,AF'
13C1 08        EX      AF,AF'
13C2 08        EX      AF,AF'
13C3 48        LD      C,B
13C4 3000      JR      NC,&13C6         ; (+&00)
13C6 40        LD      B,B
13C7 48        LD      C,B
13C8 50        LD      D,B
13C9 60        LD      H,B
13CA 50        LD      D,B
13CB 48        LD      C,B
13CC 00        NOP     
13CD 00        NOP     
13CE 3010      JR      NC,&13E0         ; (+&10)
13D0 1010      DJNZ    &13E2            ; (+&10)
13D2 1038      DJNZ    &140C            ; (+&38)
13D4 00        NOP     
13D5 00        NOP     
13D6 00        NOP     
13D7 68        LD      L,B
13D8 54        LD      D,H
13D9 54        LD      D,H
13DA 54        LD      D,H
13DB 54        LD      D,H
13DC 00        NOP     
13DD 00        NOP     
13DE 00        NOP     
13DF 50        LD      D,B
13E0 68        LD      L,B
13E1 48        LD      C,B
13E2 48        LD      C,B
13E3 48        LD      C,B
13E4 00        NOP     
13E5 00        NOP     
13E6 00        NOP     
13E7 3048      JR      NC,&1431         ; (+&48)
13E9 48        LD      C,B
13EA 48        LD      C,B
13EB 3000      JR      NC,&13ED         ; (+&00)
13ED 00        NOP     
13EE 00        NOP     
13EF 3048      JR      NC,&1439         ; (+&48)
13F1 48        LD      C,B
13F2 70        LD      (HL),B
13F3 40        LD      B,B
13F4 40        LD      B,B
13F5 00        NOP     
13F6 00        NOP     
13F7 3048      JR      NC,&1441         ; (+&48)
13F9 48        LD      C,B
13FA 380C      JR      C,&1408          ; (+&0c)
13FC 08        EX      AF,AF'
13FD 00        NOP     
13FE 00        NOP     
13FF 50        LD      D,B
1400 68        LD      L,B
1401 40        LD      B,B
1402 40        LD      B,B
1403 40        LD      B,B
1404 00        NOP     
1405 00        NOP     
1406 00        NOP     
1407 3840      JR      C,&1449          ; (+&40)
1409 3008      JR      NC,&1413         ; (+&08)
140B 70        LD      (HL),B
140C 00        NOP     
140D 00        NOP     
140E 2070      JR      NZ,&1480         ; (+&70)
1410 2020      JR      NZ,&1432         ; (+&20)
1412 2810      JR      Z,&1424          ; (+&10)
1414 00        NOP     
1415 00        NOP     
1416 00        NOP     
1417 48        LD      C,B
1418 48        LD      C,B
1419 48        LD      C,B
141A 58        LD      E,B
141B 2800      JR      Z,&141D          ; (+&00)
141D 00        NOP     
141E 00        NOP     
141F 48        LD      C,B
1420 48        LD      C,B
1421 48        LD      C,B
1422 3030      JR      NC,&1454         ; (+&30)
1424 00        NOP     
1425 00        NOP     
1426 00        NOP     
1427 54        LD      D,H
1428 54        LD      D,H
1429 54        LD      D,H
142A 54        LD      D,H
142B 2800      JR      Z,&142D          ; (+&00)
142D 00        NOP     
142E 00        NOP     
142F 44        LD      B,H
1430 2810      JR      Z,&1442          ; (+&10)
1432 2844      JR      Z,&1478          ; (+&44)
1434 00        NOP     
1435 00        NOP     
1436 00        NOP     
1437 48        LD      C,B
1438 48        LD      C,B
1439 3808      JR      C,&1443          ; (+&08)
143B 48        LD      C,B
143C 3000      JR      NC,&143E         ; (+&00)
143E 00        NOP     
143F 78        LD      A,B
1440 1020      DJNZ    &1462            ; (+&20)
1442 40        LD      B,B
1443 78        LD      A,B
1444 00        NOP     
1445 00        NOP     
1446 40        LD      B,B
1447 40        LD      B,B
1448 48        LD      C,B
1449 183C      JR      &1487            ; (+&3c)
144B 08        EX      AF,AF'
144C 00        NOP     
144D 00        NOP     
144E 2828      JR      Z,&1478          ; (+&28)
1450 2828      JR      Z,&147A          ; (+&28)
1452 2828      JR      Z,&147C          ; (+&28)
1454 00        NOP     
1455 00        NOP     
1456 70        LD      (HL),B
1457 2010      JR      NZ,&1469         ; (+&10)
1459 68        LD      L,B
145A 183C      JR      &1498            ; (+&3c)
145C 08        EX      AF,AF'
145D 00        NOP     
145E 00        NOP     
145F 1000      DJNZ    &1461            ; (+&00)
1461 7C        LD      A,H
1462 00        NOP     
1463 1000      DJNZ    &1465            ; (+&00)
1465 7C        LD      A,H
1466 7C        LD      A,H
1467 7C        LD      A,H
1468 7C        LD      A,H
1469 7C        LD      A,H
146A 7C        LD      A,H
146B 7C        LD      A,H
146C 7C        LD      A,H
146D 00        NOP     
146E 00        NOP     
146F 00        NOP     
1470 00        NOP     
1471 00        NOP     
1472 00        NOP     
1473 00        NOP     
1474 00        NOP     
1475 60        LD      H,B
1476 80        ADD     A,B
1477 40        LD      B,B
1478 20D8      JR      NZ,&1452         ; (-&28)
147A 14        INC     D
147B 14        INC     D
147C 1800      JR      &147E            ; (+&00)
147E 00        NOP     
147F 00        NOP     
1480 00        NOP     
1481 00        NOP     
1482 00        NOP     
1483 00        NOP     
1484 00        NOP     
1485 00        NOP     
1486 00        NOP     
1487 00        NOP     
1488 00        NOP     
1489 00        NOP     
148A 00        NOP     
148B 00        NOP     
148C 00        NOP     
148D 00        NOP     
148E 10F8      DJNZ    &1488            ; (-&08)
1490 04        INC     B
1491 F8        RET     M

1492 1000      DJNZ    &1494            ; (+&00)
1494 00        NOP     
1495 00        NOP     
1496 00        NOP     
1497 00        NOP     
1498 00        NOP     
1499 00        NOP     
149A 00        NOP     
149B 00        NOP     
149C 00        NOP     
149D 00        NOP     
149E 44        LD      B,H
149F 24        INC     H
14A0 F42444    CALL    P,&4424
14A3 00        NOP     
14A4 00        NOP     
14A5 80        ADD     A,B
14A6 E0        RET     PO

14A7 A0        AND     B
14A8 E0        RET     PO

14A9 1010      DJNZ    &14BB            ; (+&10)
14AB 101C      DJNZ    &14C9            ; (+&1c)
14AD 00        NOP     
14AE 207C      JR      NZ,&152C         ; (+&7c)
14B0 80        ADD     A,B
14B1 7C        LD      A,H
14B2 2000      JR      NZ,&14B4         ; (+&00)
14B4 00        NOP     
14B5 80        ADD     A,B
14B6 C0        RET     NZ

14B7 A0        AND     B
14B8 A0        AND     B
14B9 1C        INC     E
14BA 08        EX      AF,AF'
14BB 08        EX      AF,AF'
14BC 08        EX      AF,AF'
14BD 50        LD      D,B
14BE 50        LD      D,B
14BF 50        LD      D,B
14C0 D8        RET     C

14C1 50        LD      D,B
14C2 2000      JR      NZ,&14C4         ; (+&00)
14C4 00        NOP     
14C5 2050      JR      NZ,&1517         ; (+&50)
14C7 D8        RET     C

14C8 50        LD      D,B
14C9 50        LD      D,B
14CA 50        LD      D,B
14CB 00        NOP     
14CC 00        NOP     
14CD E0        RET     PO

14CE 80        ADD     A,B
14CF C0        RET     NZ

14D0 80        ADD     A,B
14D1 3820      JR      C,&14F3          ; (+&20)
14D3 3020      JR      NC,&14F5         ; (+&20)
14D5 E0        RET     PO

14D6 80        ADD     A,B
14D7 80        ADD     A,B
14D8 E0        RET     PO

14D9 1C        INC     E
14DA 14        INC     D
14DB 1814      JR      &14F1            ; (+&14)
14DD 80        ADD     A,B
14DE C0        RET     NZ

14DF E0        RET     PO

14E0 40        LD      B,B
14E1 1C        INC     E
14E2 14        INC     D
14E3 14        INC     D
14E4 1C        INC     E
14E5 E0        RET     PO

14E6 2040      JR      NZ,&1528         ; (+&40)
14E8 20EC      JR      NZ,&14D6         ; (-&14)
14EA 04        INC     B
14EB 08        EX      AF,AF'
14EC 0C        INC     C
14ED 00        NOP     
14EE 00        NOP     
14EF 00        NOP     
14F0 00        NOP     
14F1 00        NOP     
14F2 00        NOP     
14F3 00        NOP     
14F4 00        NOP     
14F5 E0        RET     PO

14F6 80        ADD     A,B
14F7 80        ADD     A,B
14F8 E0        RET     PO

14F9 1C        INC     E
14FA 14        INC     D
14FB 14        INC     D
14FC 1C        INC     E
14FD E0        RET     PO

14FE A0        AND     B
14FF E0        RET     PO

1500 80        ADD     A,B
1501 9C        SBC     A,H
1502 14        INC     D
1503 14        INC     D
1504 1C        INC     E
1505 E0        RET     PO

1506 A0        AND     B
1507 E0        RET     PO

1508 80        ADD     A,B
1509 9C        SBC     A,H
150A 1018      DJNZ    &1524            ; (+&18)
150C 10E0      DJNZ    &14EE            ; (-&20)
150E 80        ADD     A,B
150F 80        ADD     A,B
1510 E0        RET     PO

1511 1C        INC     E
1512 1018      DJNZ    &152C            ; (+&18)
1514 10C0      DJNZ    &14D6            ; (-&40)
1516 A0        AND     B
1517 A0        AND     B
1518 C0        RET     NZ

1519 1010      DJNZ    &152B            ; (+&10)
151B 101C      DJNZ    &1539            ; (+&1c)
151D C0        RET     NZ

151E A0        AND     B
151F A0        AND     B
1520 CC1008    CALL    Z,&0810
1523 04        INC     B
1524 1800      JR      &1526            ; (+&00)
1526 00        NOP     
1527 00        NOP     
1528 00        NOP     
1529 00        NOP     
152A 00        NOP     
152B 00        NOP     
152C 00        NOP     
152D C0        RET     NZ

152E A0        AND     B
152F A0        AND     B
1530 C0        RET     NZ

1531 1C        INC     E
1532 14        INC     D
1533 1814      JR      &1549            ; (+&14)
1535 00        NOP     
1536 88        ADC     A,B
1537 90        SUB     B
1538 BC        CP      H
1539 90        SUB     B
153A 88        ADC     A,B
153B 00        NOP     
153C 00        NOP     
153D 20A8      JR      NZ,&14E7         ; (-&58)
153F 70        LD      (HL),B
1540 2000      JR      NZ,&1542         ; (+&00)
1542 D8        RET     C

1543 D8        RET     C

1544 00        NOP     
1545 E0        RET     PO

1546 80        ADD     A,B
1547 C0        RET     NZ

1548 80        ADD     A,B
1549 EC0808    CALL    PE,&0808
154C 0C        INC     C
154D 00        NOP     
154E 00        NOP     
154F 00        NOP     
1550 00        NOP     
1551 00        NOP     
1552 00        NOP     
1553 00        NOP     
1554 00        NOP     
1555 00        NOP     
1556 00        NOP     
1557 00        NOP     
1558 00        NOP     
1559 00        NOP     
155A 00        NOP     
155B 00        NOP     
155C 00        NOP     
155D 00        NOP     
155E F0        RET     P

155F C0        RET     NZ

1560 A0        AND     B
1561 90        SUB     B
1562 08        EX      AF,AF'
1563 04        INC     B
1564 00        NOP     
1565 00        NOP     
1566 00        NOP     
1567 00        NOP     
1568 00        NOP     
1569 00        NOP     
156A 00        NOP     
156B 00        NOP     
156C 00        NOP     
156D FF        RST     &38
156E FF        RST     &38
156F FF        RST     &38
1570 FF        RST     &38
1571 FF        RST     &38
1572 FF        RST     &38
1573 FF        RST     &38
1574 FF        RST     &38
1575 00        NOP     
1576 00        NOP     
1577 00        NOP     
1578 00        NOP     
1579 00        NOP     
157A 00        NOP     
157B 00        NOP     
157C FF        RST     &38
157D 00        NOP     
157E 00        NOP     
157F 00        NOP     
1580 00        NOP     
1581 00        NOP     
1582 00        NOP     
1583 FF        RST     &38
1584 00        NOP     
1585 00        NOP     
1586 00        NOP     
1587 00        NOP     
1588 00        NOP     
1589 00        NOP     
158A FF        RST     &38
158B 00        NOP     
158C 00        NOP     
158D 00        NOP     
158E 00        NOP     
158F 00        NOP     
1590 00        NOP     
1591 FF        RST     &38
1592 00        NOP     
1593 00        NOP     
1594 00        NOP     
1595 010101    LD      BC,&0101
1598 010101    LD      BC,&0101
159B 010102    LD      BC,&0201
159E 02        LD      (BC),A
159F 02        LD      (BC),A
15A0 02        LD      (BC),A
15A1 02        LD      (BC),A
15A2 02        LD      (BC),A
15A3 02        LD      (BC),A
15A4 02        LD      (BC),A
15A5 04        INC     B
15A6 04        INC     B
15A7 04        INC     B
15A8 04        INC     B
15A9 04        INC     B
15AA 04        INC     B
15AB 04        INC     B
15AC 04        INC     B
15AD 08        EX      AF,AF'
15AE 08        EX      AF,AF'
15AF 08        EX      AF,AF'
15B0 08        EX      AF,AF'
15B1 08        EX      AF,AF'
15B2 08        EX      AF,AF'
15B3 08        EX      AF,AF'
15B4 08        EX      AF,AF'
15B5 00        NOP     
15B6 00        NOP     
15B7 00        NOP     
15B8 F0        RET     P

15B9 1010      DJNZ    &15CB            ; (+&10)
15BB 1010      DJNZ    &15CD            ; (+&10)
15BD 010304    LD      BC,&0403
15C0 0C        INC     C
15C1 13        INC     DE
15C2 33        INC     SP
15C3 4C        LD      C,H
15C4 CC0000    CALL    Z,&0000
15C7 00        NOP     
15C8 00        NOP     
15C9 33        INC     SP
15CA 33        INC     SP
15CB CCCC00    CALL    Z,&00CC
15CE 00        NOP     
15CF 00        NOP     
15D0 03        INC     BC
15D1 04        INC     B
15D2 08        EX      AF,AF'
15D3 1010      DJNZ    &15E5            ; (+&10)
15D5 FF        RST     &38
15D6 80        ADD     A,B
15D7 80        ADD     A,B
15D8 80        ADD     A,B
15D9 80        ADD     A,B
15DA 80        ADD     A,B
15DB 80        ADD     A,B
15DC 80        ADD     A,B
15DD 1010      DJNZ    &15EF            ; (+&10)
15DF 20C0      JR      NZ,&15A1         ; (-&40)
15E1 00        NOP     
15E2 00        NOP     
15E3 00        NOP     
15E4 00        NOP     
15E5 08        EX      AF,AF'
15E6 08        EX      AF,AF'
15E7 04        INC     B
15E8 03        INC     BC
15E9 00        NOP     
15EA 00        NOP     
15EB 00        NOP     
15EC 00        NOP     
15ED 08        EX      AF,AF'
15EE 08        EX      AF,AF'
15EF 08        EX      AF,AF'
15F0 0F        RRCA    
15F1 00        NOP     
15F2 00        NOP     
15F3 00        NOP     
15F4 00        NOP     
15F5 FF        RST     &38
15F6 00        NOP     
15F7 00        NOP     
15F8 00        NOP     
15F9 00        NOP     
15FA 00        NOP     
15FB 00        NOP     
15FC 00        NOP     
15FD 00        NOP     
15FE FF        RST     &38
15FF 00        NOP     
1600 00        NOP     
1601 00        NOP     
1602 00        NOP     
1603 00        NOP     
1604 00        NOP     
1605 00        NOP     
1606 00        NOP     
1607 FF        RST     &38
1608 00        NOP     
1609 00        NOP     
160A 00        NOP     
160B 00        NOP     
160C 00        NOP     
160D 00        NOP     
160E 00        NOP     
160F 00        NOP     
1610 FF        RST     &38
1611 00        NOP     
1612 00        NOP     
1613 00        NOP     
1614 00        NOP     
1615 80        ADD     A,B
1616 80        ADD     A,B
1617 80        ADD     A,B
1618 80        ADD     A,B
1619 80        ADD     A,B
161A 80        ADD     A,B
161B 80        ADD     A,B
161C 80        ADD     A,B
161D 40        LD      B,B
161E 40        LD      B,B
161F 40        LD      B,B
1620 40        LD      B,B
1621 40        LD      B,B
1622 40        LD      B,B
1623 40        LD      B,B
1624 40        LD      B,B
1625 2020      JR      NZ,&1647         ; (+&20)
1627 2020      JR      NZ,&1649         ; (+&20)
1629 2020      JR      NZ,&164B         ; (+&20)
162B 2020      JR      NZ,&164D         ; (+&20)
162D 1010      DJNZ    &163F            ; (+&10)
162F 1010      DJNZ    &1641            ; (+&10)
1631 1010      DJNZ    &1643            ; (+&10)
1633 1010      DJNZ    &1645            ; (+&10)
1635 1010      DJNZ    &1647            ; (+&10)
1637 10F0      DJNZ    &1629            ; (-&10)
1639 00        NOP     
163A 00        NOP     
163B 00        NOP     
163C 00        NOP     
163D 33        INC     SP
163E 32CCC8    LD      (&C8CC),A
1641 3020      JR      NC,&1663         ; (+&20)
1643 C0        RET     NZ

1644 80        ADD     A,B
1645 33        INC     SP
1646 33        INC     SP
1647 CCCC00    CALL    Z,&00CC
164A 00        NOP     
164B 00        NOP     
164C 00        NOP     
164D 1010      DJNZ    &165F            ; (+&10)
164F 08        EX      AF,AF'
1650 04        INC     B
1651 03        INC     BC
1652 00        NOP     
1653 00        NOP     
1654 00        NOP     
1655 80        ADD     A,B
1656 80        ADD     A,B
1657 80        ADD     A,B
1658 80        ADD     A,B
1659 80        ADD     A,B
165A 80        ADD     A,B
165B 80        ADD     A,B
165C FF        RST     &38
165D 00        NOP     
165E 00        NOP     
165F 00        NOP     
1660 00        NOP     
1661 C0        RET     NZ

1662 2010      JR      NZ,&1674         ; (+&10)
1664 1000      DJNZ    &1666            ; (+&00)
1666 00        NOP     
1667 00        NOP     
1668 00        NOP     
1669 03        INC     BC
166A 04        INC     B
166B 08        EX      AF,AF'
166C 08        EX      AF,AF'
166D 00        NOP     
166E 00        NOP     
166F 00        NOP     
1670 00        NOP     
1671 0F        RRCA    
1672 08        EX      AF,AF'
1673 08        EX      AF,AF'
1674 08        EX      AF,AF'
1675 03        INC     BC
1676 03        INC     BC
1677 03        INC     BC
1678 03        INC     BC
1679 03        INC     BC
167A 03        INC     BC
167B 03        INC     BC
167C 03        INC     BC
167D 80        ADD     A,B
167E 80        ADD     A,B
167F 40        LD      B,B
1680 40        LD      B,B
1681 2010      JR      NZ,&1693         ; (+&10)
1683 0C        INC     C
1684 03        INC     BC
1685 010102    LD      BC,&0201
1688 02        LD      (BC),A
1689 02        LD      (BC),A
168A 04        INC     B
168B 04        INC     B
168C 08        EX      AF,AF'
168D 0F        RRCA    
168E 0F        RRCA    
168F 0F        RRCA    
1690 0F        RRCA    
1691 0F        RRCA    
1692 0F        RRCA    
1693 0F        RRCA    
1694 0F        RRCA    
1695 02        LD      (BC),A
1696 02        LD      (BC),A
1697 02        LD      (BC),A
1698 02        LD      (BC),A
1699 02        LD      (BC),A
169A 02        LD      (BC),A
169B 07        RLCA    
169C 02        LD      (BC),A
169D 00        NOP     
169E 00        NOP     
169F 00        NOP     
16A0 00        NOP     
16A1 00        NOP     
16A2 00        NOP     
16A3 FF        RST     &38
16A4 FF        RST     &38
16A5 00        NOP     
16A6 00        NOP     
16A7 00        NOP     
16A8 00        NOP     
16A9 00        NOP     
16AA FF        RST     &38
16AB FF        RST     &38
16AC FF        RST     &38
16AD 00        NOP     
16AE 00        NOP     
16AF 00        NOP     
16B0 00        NOP     
16B1 FF        RST     &38
16B2 FF        RST     &38
16B3 FF        RST     &38
16B4 FF        RST     &38
16B5 C0        RET     NZ

16B6 300C      JR      NC,&16C4         ; (+&0c)
16B8 03        INC     BC
16B9 00        NOP     
16BA 00        NOP     
16BB 00        NOP     
16BC 00        NOP     
16BD 0F        RRCA    
16BE 0F        RRCA    
16BF 0F        RRCA    
16C0 0F        RRCA    
16C1 F0        RET     P

16C2 F0        RET     P

16C3 F0        RET     P

16C4 F0        RET     P

16C5 33        INC     SP
16C6 33        INC     SP
16C7 CCCC33    CALL    Z,&33CC
16CA 33        INC     SP
16CB CCCC03    CALL    Z,&03CC
16CE 03        INC     BC
16CF 0C        INC     C
16D0 0C        INC     C
16D1 03        INC     BC
16D2 03        INC     BC
16D3 0C        INC     C
16D4 0C        INC     C
16D5 08        EX      AF,AF'
16D6 08        EX      AF,AF'
16D7 1020      DJNZ    &16F9            ; (+&20)
16D9 C0        RET     NZ

16DA 00        NOP     
16DB 00        NOP     
16DC 00        NOP     
16DD 010102    LD      BC,&0201
16E0 02        LD      (BC),A
16E1 04        INC     B
16E2 08        EX      AF,AF'
16E3 30C0      JR      NC,&16A5         ; (-&40)
16E5 00        NOP     
16E6 66        LD      H,(HL)
16E7 66        LD      H,(HL)
16E8 1818      JR      &1702            ; (+&18)
16EA 66        LD      H,(HL)
16EB 66        LD      H,(HL)
16EC 00        NOP     
16ED 44        LD      B,H
16EE 44        LD      B,H
16EF 44        LD      B,H
16F0 44        LD      B,H
16F1 44        LD      B,H
16F2 44        LD      B,H
16F3 44        LD      B,H
16F4 44        LD      B,H
16F5 1010      DJNZ    &1707            ; (+&10)
16F7 10F0      DJNZ    &16E9            ; (-&10)
16F9 1010      DJNZ    &170B            ; (+&10)
16FB 1010      DJNZ    &170D            ; (+&10)
16FD 02        LD      (BC),A
16FE 02        LD      (BC),A
16FF 02        LD      (BC),A
1700 02        LD      (BC),A
1701 02        LD      (BC),A
1702 02        LD      (BC),A
1703 FF        RST     &38
1704 02        LD      (BC),A
1705 07        RLCA    
1706 07        RLCA    
1707 07        RLCA    
1708 07        RLCA    
1709 07        RLCA    
170A 07        RLCA    
170B 07        RLCA    
170C 07        RLCA    
170D 80        ADD     A,B
170E 80        ADD     A,B
170F 40        LD      B,B
1710 40        LD      B,B
1711 2020      JR      NZ,&1733         ; (+&20)
1713 1010      DJNZ    &1725            ; (+&10)
1715 00        NOP     
1716 00        NOP     
1717 00        NOP     
1718 00        NOP     
1719 03        INC     BC
171A 0C        INC     C
171B 30C0      JR      NC,&16DD         ; (-&40)
171D 08        EX      AF,AF'
171E 08        EX      AF,AF'
171F 04        INC     B
1720 04        INC     B
1721 02        LD      (BC),A
1722 02        LD      (BC),A
1723 010110    LD      BC,&1001
1726 1010      DJNZ    &1738            ; (+&10)
1728 FF        RST     &38
1729 00        NOP     
172A 00        NOP     
172B 00        NOP     
172C 00        NOP     
172D C0        RET     NZ

172E 3806      JR      C,&1736          ; (+&06)
1730 010000    LD      BC,&0000
1733 00        NOP     
1734 00        NOP     
1735 010102    LD      BC,&0201
1738 02        LD      (BC),A
1739 04        INC     B
173A 04        INC     B
173B 08        EX      AF,AF'
173C 08        EX      AF,AF'
173D 03        INC     BC
173E 1C        INC     E
173F 60        LD      H,B
1740 80        ADD     A,B
1741 00        NOP     
1742 00        NOP     
1743 00        NOP     
1744 00        NOP     
1745 44        LD      B,H
1746 FF        RST     &38
1747 44        LD      B,H
1748 44        LD      B,H
1749 44        LD      B,H
174A FF        RST     &38
174B 44        LD      B,H
174C 44        LD      B,H
174D 80        ADD     A,B
174E 40        LD      B,B
174F 2010      JR      NZ,&1761         ; (+&10)
1751 08        EX      AF,AF'
1752 04        INC     B
1753 02        LD      (BC),A
1754 013333    LD      BC,&3333
1757 0C        INC     C
1758 0C        INC     C
1759 03        INC     BC
175A 03        INC     BC
175B 00        NOP     
175C 00        NOP     
175D 010101    LD      BC,&0101
1760 010101    LD      BC,&0101
1763 01FF11    LD      BC,&11FF
1766 224488    LD      (&8844),HL
1769 112244    LD      DE,&4422
176C 88        ADC     A,B
176D 88        ADC     A,B
176E 44        LD      B,H
176F 221188    LD      (&8811),HL
1772 44        LD      B,H
1773 2211C0    LD      (&C011),HL
1776 C0        RET     NZ

1777 C0        RET     NZ

1778 C0        RET     NZ

1779 C0        RET     NZ

177A C0        RET     NZ

177B C0        RET     NZ

177C C0        RET     NZ

177D 03        INC     BC
177E 0C        INC     C
177F 1020      DJNZ    &17A1            ; (+&20)
1781 40        LD      B,B
1782 40        LD      B,B
1783 80        ADD     A,B
1784 80        ADD     A,B
1785 80        ADD     A,B
1786 80        ADD     A,B
1787 40        LD      B,B
1788 40        LD      B,B
1789 40        LD      B,B
178A 2020      JR      NZ,&17AC         ; (+&20)
178C 10F0      DJNZ    &177E            ; (-&10)
178E F0        RET     P

178F F0        RET     P

1790 F0        RET     P

1791 F0        RET     P

1792 F0        RET     P

1793 F0        RET     P

1794 F0        RET     P

1795 00        NOP     
1796 00        NOP     
1797 00        NOP     
1798 00        NOP     
1799 00        NOP     
179A 02        LD      (BC),A
179B FF        RST     &38
179C 02        LD      (BC),A
179D FF        RST     &38
179E FF        RST     &38
179F 00        NOP     
17A0 00        NOP     
17A1 00        NOP     
17A2 00        NOP     
17A3 00        NOP     
17A4 00        NOP     
17A5 FF        RST     &38
17A6 FF        RST     &38
17A7 FF        RST     &38
17A8 00        NOP     
17A9 00        NOP     
17AA 00        NOP     
17AB 00        NOP     
17AC 00        NOP     
17AD FF        RST     &38
17AE FF        RST     &38
17AF FF        RST     &38
17B0 FF        RST     &38
17B1 00        NOP     
17B2 00        NOP     
17B3 00        NOP     
17B4 00        NOP     
17B5 03        INC     BC
17B6 0C        INC     C
17B7 30C0      JR      NC,&1779         ; (-&40)
17B9 00        NOP     
17BA 00        NOP     
17BB 00        NOP     
17BC 00        NOP     
17BD F0        RET     P

17BE F0        RET     P

17BF F0        RET     P

17C0 F0        RET     P

17C1 0F        RRCA    
17C2 0F        RRCA    
17C3 0F        RRCA    
17C4 0F        RRCA    
17C5 CCCC33    CALL    Z,&33CC
17C8 33        INC     SP
17C9 CCCC33    CALL    Z,&33CC
17CC 33        INC     SP
17CD 3030      JR      NC,&17FF         ; (+&30)
17CF C0        RET     NZ

17D0 C0        RET     NZ

17D1 3030      JR      NC,&1803         ; (+&30)
17D3 C0        RET     NZ

17D4 C0        RET     NZ

17D5 00        NOP     
17D6 00        NOP     
17D7 00        NOP     
17D8 C0        RET     NZ

17D9 2010      JR      NZ,&17EB         ; (+&10)
17DB 08        EX      AF,AF'
17DC 08        EX      AF,AF'
17DD C0        RET     NZ

17DE 3008      JR      NC,&17E8         ; (+&08)
17E0 04        INC     B
17E1 02        LD      (BC),A
17E2 02        LD      (BC),A
17E3 010181    LD      BC,&8101
17E6 42        LD      B,D
17E7 24        INC     H
17E8 1818      JR      &1802            ; (+&18)
17EA 24        INC     H
17EB 42        LD      B,D
17EC 81        ADD     A,C
17ED 00        NOP     
17EE FF        RST     &38
17EF 00        NOP     
17F0 00        NOP     
17F1 00        NOP     
17F2 FF        RST     &38
17F3 00        NOP     
17F4 00        NOP     
17F5 1010      DJNZ    &1807            ; (+&10)
17F7 101F      DJNZ    &1818            ; (+&1f)
17F9 1010      DJNZ    &180B            ; (+&10)
17FB 1010      DJNZ    &180D            ; (+&10)
17FD 1010      DJNZ    &180F            ; (+&10)
17FF 10FF      DJNZ    &1800            ; (-&01)
1801 1010      DJNZ    &1813            ; (+&10)
1803 1010      DJNZ    &1815            ; (+&10)
1805 E0        RET     PO

1806 E0        RET     PO

1807 E0        RET     PO

1808 E0        RET     PO

1809 E0        RET     PO

180A E0        RET     PO

180B E0        RET     PO

180C E0        RET     PO

180D 1010      DJNZ    &181F            ; (+&10)
180F 2020      JR      NZ,&1831         ; (+&20)
1811 40        LD      B,B
1812 40        LD      B,B
1813 80        ADD     A,B
1814 80        ADD     A,B
1815 00        NOP     
1816 00        NOP     
1817 00        NOP     
1818 00        NOP     
1819 C0        RET     NZ

181A 300C      JR      NC,&1828         ; (+&0c)
181C 03        INC     BC
181D 1020      DJNZ    &183F            ; (+&20)
181F 2040      JR      NZ,&1861         ; (+&40)
1821 40        LD      B,B
1822 40        LD      B,B
1823 80        ADD     A,B
1824 80        ADD     A,B
1825 00        NOP     
1826 00        NOP     
1827 00        NOP     
1828 FF        RST     &38
1829 1010      DJNZ    &183B            ; (+&10)
182B 1010      DJNZ    &183D            ; (+&10)
182D 00        NOP     
182E 00        NOP     
182F 00        NOP     
1830 00        NOP     
1831 010638    LD      BC,&3806
1834 C0        RET     NZ

1835 08        EX      AF,AF'
1836 08        EX      AF,AF'
1837 04        INC     B
1838 04        INC     B
1839 02        LD      (BC),A
183A 02        LD      (BC),A
183B 010100    LD      BC,&0001
183E 00        NOP     
183F 00        NOP     
1840 00        NOP     
1841 80        ADD     A,B
1842 60        LD      H,B
1843 1C        INC     E
1844 03        INC     BC
1845 99        SBC     A,C
1846 66        LD      H,(HL)
1847 66        LD      H,(HL)
1848 99        SBC     A,C
1849 99        SBC     A,C
184A 66        LD      H,(HL)
184B 66        LD      H,(HL)
184C 99        SBC     A,C
184D 010204    LD      BC,&0402
1850 08        EX      AF,AF'
1851 1020      DJNZ    &1873            ; (+&20)
1853 40        LD      B,B
1854 80        ADD     A,B
1855 00        NOP     
1856 00        NOP     
1857 C0        RET     NZ

1858 C0        RET     NZ

1859 3030      JR      NC,&188B         ; (+&30)
185B CCCCFF    CALL    Z,&FFCC
185E 010101    LD      BC,&0101
1861 010101    LD      BC,&0101
1864 01FFFF    LD      BC,&FFFF
1867 FF        RST     &38
1868 FF        RST     &38
1869 FF        RST     &38
186A FF        RST     &38
186B FF        RST     &38
186C FF        RST     &38
186D E5        PUSH    HL
186E 21A8FB    LD      HL,&FBA8
1871 3AACFB    LD      A,(&FBAC)
1874 3C        INC     A
1875 32ACFB    LD      (&FBAC),A
1878 BE        CP      (HL)
1879 3812      JR      C,&188D          ; (+&12)
187B 23        INC     HL
187C BE        CP      (HL)
187D 3812      JR      C,&1891          ; (+&12)
187F 23        INC     HL
1880 BE        CP      (HL)
1881 380A      JR      C,&188D          ; (+&0a)
1883 23        INC     HL
1884 BE        CP      (HL)
1885 380A      JR      C,&1891          ; (+&0a)
1887 AF        XOR     A
1888 32ACFB    LD      (&FBAC),A
188B 18E1      JR      &186E            ; (-&1f)
188D 3E01      LD      A,&01
188F 1803      JR      &1894            ; (+&03)
1891 AF        XOR     A
1892 20E5      JR      NZ,&1879         ; (-&1b)
1894 D5        PUSH    DE
1895 C5        PUSH    BC
1896 F5        PUSH    AF
1897 CDBA18    CALL    &18BA
189A 3817      JR      C,&18B3          ; (+&17)
189C E1        POP     HL
189D CBBF      RES     7,A
189F 1C        INC     E
18A0 25        DEC     H
18A1 2002      JR      NZ,&18A5         ; (+&02)
18A3 CBFF      SET     7,A
18A5 1D        DEC     E
18A6 2803      JR      Z,&18AB          ; (+&03)
18A8 0F        RRCA    
18A9 18FA      JR      &18A5            ; (-&06)
18AB CD451A    CALL    &1A45
18AE C1        POP     BC
18AF D1        POP     DE
18B0 E1        POP     HL
18B1 AF        XOR     A
18B2 C9        RET     

18B3 F1        POP     AF
18B4 C1        POP     BC
18B5 D1        POP     DE
18B6 E1        POP     HL
18B7 AF        XOR     A
18B8 3D        DEC     A
18B9 C9        RET     

18BA DDE5      PUSH    IX
18BC C1        POP     BC
18BD FDE5      PUSH    IY
18BF D1        POP     DE
18C0 CDF618    CALL    &18F6
18C3 D8        RET     C

18C4 21BF00    LD      HL,&00BF
18C7 ED52      SBC     HL,DE
18C9 D8        RET     C

18CA 3E07      LD      A,&07
18CC 91        SUB     C
18CD E607      AND     &07
18CF 3C        INC     A
18D0 57        LD      D,A
18D1 79        LD      A,C
18D2 E6F8      AND     &F8
18D4 4F        LD      C,A
18D5 7D        LD      A,L
18D6 E607      AND     &07
18D8 B1        OR      C
18D9 4F        LD      C,A
18DA 7D        LD      A,L
18DB 0F        RRCA    
18DC 0F        RRCA    
18DD 0F        RRCA    
18DE E61F      AND     &1F
18E0 47        LD      B,A
18E1 E6F8      AND     &F8
18E3 5F        LD      E,A
18E4 3A4EFB    LD      A,(&FB4E)
18E7 80        ADD     A,B
18E8 E607      AND     &07
18EA B3        OR      E
18EB 47        LD      B,A
18EC EF        RST     &28
18ED 1E08      LD      E,&08
18EF 0F        RRCA    
18F0 1D        DEC     E
18F1 15        DEC     D
18F2 20FB      JR      NZ,&18EF         ; (-&05)
18F4 B7        OR      A
18F5 C9        RET     

18F6 2A9CFB    LD      HL,(&FB9C)
18F9 19        ADD     HL,DE
18FA EB        EX      DE,HL
18FB 2A9AFB    LD      HL,(&FB9A)
18FE 09        ADD     HL,BC
18FF 44        LD      B,H
1900 4D        LD      C,L
1901 AF        XOR     A
1902 B8        CP      B
1903 C9        RET     

1904 DDE5      PUSH    IX
1906 C1        POP     BC
1907 FDE5      PUSH    IY
1909 D1        POP     DE
190A CDC018    CALL    &18C0
190D 3807      JR      C,&1916          ; (+&07)
190F CB7F      BIT     7,A
1911 3E00      LD      A,&00
1913 C8        RET     Z

1914 3C        INC     A
1915 C9        RET     

1916 AF        XOR     A
1917 3D        DEC     A
1918 C9        RET     

1919 CD6D18    CALL    &186D
191C E5        PUSH    HL
191D C5        PUSH    BC
191E ED4B96FB  LD      BC,(&FB96)
1922 ED5B98FB  LD      DE,(&FB98)
1926 FDE5      PUSH    IY
1928 E1        POP     HL
1929 CDA419    CALL    &19A4
192C D9        EXX     
192D 5F        LD      E,A
192E C601      ADD     A,&01
1930 9F        SBC     A,A
1931 57        LD      D,A
1932 D9        EXX     
1933 DDE5      PUSH    IX
1935 E3        EX      (SP),HL
1936 50        LD      D,B
1937 59        LD      E,C
1938 CDA419    CALL    &19A4
193B D9        EXX     
193C 4F        LD      C,A
193D C601      ADD     A,&01
193F 9F        SBC     A,A
1940 47        LD      B,A
1941 D9        EXX     
1942 D1        POP     DE
1943 B7        OR      A
1944 ED52      SBC     HL,DE
1946 19        ADD     HL,DE
1947 44        LD      B,H
1948 4D        LD      C,L
1949 210000    LD      HL,&0000
194C FA7819    JP      M,&1978
194F D5        PUSH    DE
1950 DDE5      PUSH    IX
1952 E3        EX      (SP),HL
1953 ED5B96FB  LD      DE,(&FB96)
1957 B7        OR      A
1958 ED52      SBC     HL,DE
195A E1        POP     HL
195B D1        POP     DE
195C 2843      JR      Z,&19A1          ; (+&43)
195E D9        EXX     
195F DD09      ADD     IX,BC
1961 D9        EXX     
1962 19        ADD     HL,DE
1963 E5        PUSH    HL
1964 29        ADD     HL,HL
1965 3802      JR      C,&1969          ; (+&02)
1967 ED42      SBC     HL,BC
1969 E1        POP     HL
196A 3807      JR      C,&1973          ; (+&07)
196C D9        EXX     
196D FD19      ADD     IY,DE
196F D9        EXX     
1970 B7        OR      A
1971 ED42      SBC     HL,BC
1973 CD6D18    CALL    &186D
1976 18D7      JR      &194F            ; (-&29)
1978 D5        PUSH    DE
1979 FDE5      PUSH    IY
197B E3        EX      (SP),HL
197C ED5B98FB  LD      DE,(&FB98)
1980 B7        OR      A
1981 ED52      SBC     HL,DE
1983 E1        POP     HL
1984 D1        POP     DE
1985 281A      JR      Z,&19A1          ; (+&1a)
1987 D9        EXX     
1988 FD19      ADD     IY,DE
198A D9        EXX     
198B 09        ADD     HL,BC
198C E5        PUSH    HL
198D 29        ADD     HL,HL
198E 3802      JR      C,&1992          ; (+&02)
1990 ED52      SBC     HL,DE
1992 E1        POP     HL
1993 3807      JR      C,&199C          ; (+&07)
1995 D9        EXX     
1996 DD09      ADD     IX,BC
1998 D9        EXX     
1999 B7        OR      A
199A ED52      SBC     HL,DE
199C CD6D18    CALL    &186D
199F 18D7      JR      &1978            ; (-&29)
19A1 C1        POP     BC
19A2 E1        POP     HL
19A3 C9        RET     

19A4 EB        EX      DE,HL
19A5 AF        XOR     A
19A6 ED52      SBC     HL,DE
19A8 C8        RET     Z

19A9 19        ADD     HL,DE
19AA F2B119    JP      P,&19B1
19AD EB        EX      DE,HL
19AE 3D        DEC     A
19AF 1801      JR      &19B2            ; (+&01)
19B1 3C        INC     A
19B2 B7        OR      A
19B3 ED52      SBC     HL,DE
19B5 C9        RET     

19B6 21FDFF    LD      HL,&FFFD
19B9 22AEFB    LD      (&FBAE),HL
19BC CD991F    CALL    &1F99
19BF CDEF1A    CALL    &1AEF
19C2 DD2298FB  LD      (&FB98),IX
19C6 CDF819    CALL    &19F8
19C9 FD23      INC     IY
19CB DDE5      PUSH    IX
19CD CD591A    CALL    &1A59
19D0 DDE1      POP     IX
19D2 D8        RET     C

19D3 FD2B      DEC     IY
19D5 FD2B      DEC     IY
19D7 CD591A    CALL    &1A59
19DA D8        RET     C

19DB ED7365FB  LD      (&FB65),SP
19DF 2AAEFB    LD      HL,(&FBAE)
19E2 F9        LD      SP,HL
19E3 DDE1      POP     IX
19E5 FDE1      POP     IY
19E7 ED73AEFB  LD      (&FBAE),SP
19EB ED7B65FB  LD      SP,(&FB65)
19EF 11FDFF    LD      DE,&FFFD
19F2 B7        OR      A
19F3 ED52      SBC     HL,DE
19F5 20C8      JR      NZ,&19BF         ; (-&38)
19F7 C9        RET     

19F8 CDBA18    CALL    &18BA
19FB CD501A    CALL    &1A50
19FE CBFF      SET     7,A
1A00 1C        INC     E
1A01 1D        DEC     E
1A02 280E      JR      Z,&1A12          ; (+&0e)
1A04 0F        RRCA    
1A05 CBFF      SET     7,A
1A07 DD2B      DEC     IX
1A09 30F6      JR      NC,&1A01         ; (-&0a)
1A0B 37        SCF     
1A0C 1D        DEC     E
1A0D 2803      JR      Z,&1A12          ; (+&03)
1A0F 0F        RRCA    
1A10 18F9      JR      &1A0B            ; (-&07)
1A12 F5        PUSH    AF
1A13 CD421A    CALL    &1A42
1A16 F1        POP     AF
1A17 3003      JR      NC,&1A1C         ; (+&03)
1A19 DD23      INC     IX
1A1B C9        RET     

1A1C 11F8FF    LD      DE,&FFF8
1A1F 79        LD      A,C
1A20 83        ADD     A,E
1A21 4F        LD      C,A
1A22 D0        RET     NC

1A23 CD4F1A    CALL    &1A4F
1A26 2008      JR      NZ,&1A30         ; (+&08)
1A28 DD19      ADD     IX,DE
1A2A 2F        CPL     
1A2B CD421A    CALL    &1A42
1A2E 18EC      JR      &1A1C            ; (-&14)
1A30 CB47      BIT     0,A
1A32 C0        RET     NZ

1A33 1600      LD      D,&00
1A35 DD23      INC     IX
1A37 14        INC     D
1A38 37        SCF     
1A39 1F        RRA     
1A3A DD2B      DEC     IX
1A3C 30F9      JR      NC,&1A37         ; (-&07)
1A3E 07        RLCA    
1A3F 15        DEC     D
1A40 20FC      JR      NZ,&1A3E         ; (-&04)
1A42 CD501A    CALL    &1A50
1A45 F7        RST     &30
1A46 CBE8      SET     5,B
1A48 3A39FB    LD      A,(&FB39)
1A4B F7        RST     &30
1A4C CBA8      RES     5,B
1A4E C9        RET     

1A4F EF        RST     &28
1A50 E5        PUSH    HL
1A51 21ADFB    LD      HL,&FBAD
1A54 AE        XOR     (HL)
1A55 EEFF      XOR     &FF
1A57 E1        POP     HL
1A58 C9        RET     

1A59 CD0419    CALL    &1904
1A5C FEFF      CP      &FF
1A5E C8        RET     Z

1A5F 3D        DEC     A
1A60 CD501A    CALL    &1A50
1A63 2805      JR      Z,&1A6A          ; (+&05)
1A65 CDEF1A    CALL    &1AEF
1A68 1803      JR      &1A6D            ; (+&03)
1A6A CDC61A    CALL    &1AC6
1A6D ED5B98FB  LD      DE,(&FB98)
1A71 DDE5      PUSH    IX
1A73 E1        POP     HL
1A74 B7        OR      A
1A75 ED52      SBC     HL,DE
1A77 F2851A    JP      P,&1A85
1A7A CD961A    CALL    &1A96
1A7D D8        RET     C

1A7E DD23      INC     IX
1A80 CDC61A    CALL    &1AC6
1A83 18E8      JR      &1A6D            ; (-&18)
1A85 D5        PUSH    DE
1A86 DDE1      POP     IX
1A88 CD0419    CALL    &1904
1A8B FEFF      CP      &FF
1A8D C8        RET     Z

1A8E 3D        DEC     A
1A8F CD501A    CALL    &1A50
1A92 C8        RET     Z

1A93 CDEF1A    CALL    &1AEF
1A96 DDE5      PUSH    IX
1A98 D1        POP     DE
1A99 FDE5      PUSH    IY
1A9B C1        POP     BC
1A9C ED7365FB  LD      (&FB65),SP
1AA0 ED7BAEFB  LD      SP,(&FBAE)
1AA4 E1        POP     HL
1AA5 E5        PUSH    HL
1AA6 B7        OR      A
1AA7 ED52      SBC     HL,DE
1AA9 2009      JR      NZ,&1AB4         ; (+&09)
1AAB E1        POP     HL
1AAC E1        POP     HL
1AAD ED42      SBC     HL,BC
1AAF 09        ADD     HL,BC
1AB0 2802      JR      Z,&1AB4          ; (+&02)
1AB2 E5        PUSH    HL
1AB3 D5        PUSH    DE
1AB4 C5        PUSH    BC
1AB5 D5        PUSH    DE
1AB6 2108FE    LD      HL,&FE08
1AB9 B7        OR      A
1ABA ED72      SBC     HL,SP
1ABC 3F        CCF     
1ABD ED73AEFB  LD      (&FBAE),SP
1AC1 ED7B65FB  LD      SP,(&FB65)
1AC5 C9        RET     

1AC6 CDBA18    CALL    &18BA
1AC9 DD23      INC     IX
1ACB C5        PUSH    BC
1ACC CD0419    CALL    &1904
1ACF E1        POP     HL
1AD0 110800    LD      DE,&0008
1AD3 FEFF      CP      &FF
1AD5 2840      JR      Z,&1B17          ; (+&40)
1AD7 3D        DEC     A
1AD8 CD501A    CALL    &1A50
1ADB 2012      JR      NZ,&1AEF         ; (+&12)
1ADD ED42      SBC     HL,BC
1ADF 09        ADD     HL,BC
1AE0 28E7      JR      Z,&1AC9          ; (-&19)
1AE2 CD4F1A    CALL    &1A4F
1AE5 3C        INC     A
1AE6 20E3      JR      NZ,&1ACB         ; (-&1d)
1AE8 DD19      ADD     IX,DE
1AEA 79        LD      A,C
1AEB 83        ADD     A,E
1AEC 4F        LD      C,A
1AED 30F3      JR      NC,&1AE2         ; (-&0d)
1AEF CDBA18    CALL    &18BA
1AF2 DD23      INC     IX
1AF4 C5        PUSH    BC
1AF5 CD0419    CALL    &1904
1AF8 E1        POP     HL
1AF9 110800    LD      DE,&0008
1AFC FEFF      CP      &FF
1AFE 2817      JR      Z,&1B17          ; (+&17)
1B00 3D        DEC     A
1B01 CD501A    CALL    &1A50
1B04 2811      JR      Z,&1B17          ; (+&11)
1B06 ED42      SBC     HL,BC
1B08 09        ADD     HL,BC
1B09 28E7      JR      Z,&1AF2          ; (-&19)
1B0B CD4F1A    CALL    &1A4F
1B0E 20E4      JR      NZ,&1AF4         ; (-&1c)
1B10 DD19      ADD     IX,DE
1B12 79        LD      A,C
1B13 83        ADD     A,E
1B14 4F        LD      C,A
1B15 30F4      JR      NC,&1B0B         ; (-&0c)
1B17 DD2B      DEC     IX
1B19 C9        RET     

1B1A 0B        DEC     BC
1B1B 0B        DEC     BC
1B1C F5        PUSH    AF
1B1D CD531B    CALL    &1B53
1B20 AF        XOR     A
1B21 32ACFB    LD      (&FBAC),A
1B24 EB        EX      DE,HL
1B25 CD731B    CALL    &1B73
1B28 F1        POP     AF
1B29 F5        PUSH    AF
1B2A DC481B    CALL    C,&1B48
1B2D DD2A96FB  LD      IX,(&FB96)
1B31 FD2A98FB  LD      IY,(&FB98)
1B35 ED5BA6FB  LD      DE,(&FBA6)
1B39 19        ADD     HL,DE
1B3A CD731B    CALL    &1B73
1B3D CD1919    CALL    &1919
1B40 B7        OR      A
1B41 ED42      SBC     HL,BC
1B43 09        ADD     HL,BC
1B44 38E7      JR      C,&1B2D          ; (-&19)
1B46 F1        POP     AF
1B47 D0        RET     NC

1B48 DD2A9EFB  LD      IX,(&FB9E)
1B4C FD2AA0FB  LD      IY,(&FBA0)
1B50 C31919    JP      &1919
1B53 CB7A      BIT     7,D
1B55 2806      JR      Z,&1B5D          ; (+&06)
1B57 14        INC     D
1B58 14        INC     D
1B59 14        INC     D
1B5A 14        INC     D
1B5B 18F6      JR      &1B53            ; (-&0a)
1B5D CB78      BIT     7,B
1B5F 2806      JR      Z,&1B67          ; (+&06)
1B61 04        INC     B
1B62 04        INC     B
1B63 04        INC     B
1B64 04        INC     B
1B65 18F6      JR      &1B5D            ; (-&0a)
1B67 60        LD      H,B
1B68 69        LD      L,C
1B69 B7        OR      A
1B6A ED52      SBC     HL,DE
1B6C D0        RET     NC

1B6D 04        INC     B
1B6E 04        INC     B
1B6F 04        INC     B
1B70 04        INC     B
1B71 18F4      JR      &1B67            ; (-&0c)
1B73 E5        PUSH    HL
1B74 C5        PUSH    BC
1B75 110004    LD      DE,&0400
1B78 B7        OR      A
1B79 ED52      SBC     HL,DE
1B7B 30FC      JR      NC,&1B79         ; (-&04)
1B7D 19        ADD     HL,DE
1B7E 110002    LD      DE,&0200
1B81 CDA419    CALL    &19A4
1B84 F5        PUSH    AF
1B85 EB        EX      DE,HL
1B86 210001    LD      HL,&0100
1B89 CDA419    CALL    &19A4
1B8C C1        POP     BC
1B8D 4F        LD      C,A
1B8E EB        EX      DE,HL
1B8F 210001    LD      HL,&0100
1B92 B7        OR      A
1B93 ED52      SBC     HL,DE
1B95 EB        EX      DE,HL
1B96 D5        PUSH    DE
1B97 C5        PUSH    BC
1B98 79        LD      A,C
1B99 ED5B9EFB  LD      DE,(&FB9E)
1B9D ED4BA2FB  LD      BC,(&FBA2)
1BA1 CDBB1B    CALL    &1BBB
1BA4 2296FB    LD      (&FB96),HL
1BA7 C1        POP     BC
1BA8 E1        POP     HL
1BA9 78        LD      A,B
1BAA ED5BA0FB  LD      DE,(&FBA0)
1BAE ED4BA4FB  LD      BC,(&FBA4)
1BB2 CDBB1B    CALL    &1BBB
1BB5 2298FB    LD      (&FB98),HL
1BB8 C1        POP     BC
1BB9 E1        POP     HL
1BBA C9        RET     

1BBB D5        PUSH    DE
1BBC F5        PUSH    AF
1BBD 24        INC     H
1BBE 25        DEC     H
1BBF 2026      JR      NZ,&1BE7         ; (+&26)
1BC1 2C        INC     L
1BC2 2D        DEC     L
1BC3 110000    LD      DE,&0000
1BC6 2814      JR      Z,&1BDC          ; (+&14)
1BC8 EB        EX      DE,HL
1BC9 21EB1B    LD      HL,&1BEB
1BCC 19        ADD     HL,DE
1BCD 1E00      LD      E,&00
1BCF 56        LD      D,(HL)
1BD0 14        INC     D
1BD1 15        DEC     D
1BD2 2813      JR      Z,&1BE7          ; (+&13)
1BD4 CD600E    CALL    &0E60
1BD7 CB7C      BIT     7,H
1BD9 2801      JR      Z,&1BDC          ; (+&01)
1BDB 13        INC     DE
1BDC F1        POP     AF
1BDD B7        OR      A
1BDE E1        POP     HL
1BDF FAE41B    JP      M,&1BE4
1BE2 19        ADD     HL,DE
1BE3 C9        RET     

1BE4 ED52      SBC     HL,DE
1BE6 C9        RET     

1BE7 50        LD      D,B
1BE8 59        LD      E,C
1BE9 18F1      JR      &1BDC            ; (-&0f)
1BEB 00        NOP     
1BEC 02        LD      (BC),A
1BED 03        INC     BC
1BEE 05        DEC     B
1BEF 0608      LD      B,&08
1BF1 09        ADD     HL,BC
1BF2 0B        DEC     BC
1BF3 0D        DEC     C
1BF4 0E10      LD      C,&10
1BF6 111314    LD      DE,&1413
1BF9 1618      LD      D,&18
1BFB 19        ADD     HL,DE
1BFC 1B        DEC     DE
1BFD 1C        INC     E
1BFE 1E1F      LD      E,&1F
1C00 212224    LD      HL,&2422
1C03 2627      LD      H,&27
1C05 29        ADD     HL,HL
1C06 2A2C2D    LD      HL,(&2D2C)
1C09 2F        CPL     
1C0A 3032      JR      NC,&1C3E         ; (+&32)
1C0C 33        INC     SP
1C0D 35        DEC     (HL)
1C0E 37        SCF     
1C0F 383A      JR      C,&1C4B          ; (+&3a)
1C11 3B        DEC     SP
1C12 3D        DEC     A
1C13 3E40      LD      A,&40
1C15 41        LD      B,C
1C16 43        LD      B,E
1C17 44        LD      B,H
1C18 46        LD      B,(HL)
1C19 47        LD      B,A
1C1A 49        LD      C,C
1C1B 4A        LD      C,D
1C1C 4C        LD      C,H
1C1D 4D        LD      C,L
1C1E 4F        LD      C,A
1C1F 50        LD      D,B
1C20 52        LD      D,D
1C21 53        LD      D,E
1C22 55        LD      D,L
1C23 56        LD      D,(HL)
1C24 58        LD      E,B
1C25 59        LD      E,C
1C26 5B        LD      E,E
1C27 5C        LD      E,H
1C28 5E        LD      E,(HL)
1C29 5F        LD      E,A
1C2A 61        LD      H,C
1C2B 62        LD      H,D
1C2C 63        LD      H,E
1C2D 65        LD      H,L
1C2E 66        LD      H,(HL)
1C2F 68        LD      L,B
1C30 69        LD      L,C
1C31 6B        LD      L,E
1C32 6C        LD      L,H
1C33 6D        LD      L,L
1C34 6F        LD      L,A
1C35 70        LD      (HL),B
1C36 72        LD      (HL),D
1C37 73        LD      (HL),E
1C38 75        LD      (HL),L
1C39 76        HALT    
1C3A 77        LD      (HL),A
1C3B 79        LD      A,C
1C3C 7A        LD      A,D
1C3D 7B        LD      A,E
1C3E 7D        LD      A,L
1C3F 7E        LD      A,(HL)
1C40 80        ADD     A,B
1C41 81        ADD     A,C
1C42 82        ADD     A,D
1C43 84        ADD     A,H
1C44 85        ADD     A,L
1C45 86        ADD     A,(HL)
1C46 88        ADC     A,B
1C47 89        ADC     A,C
1C48 8A        ADC     A,D
1C49 8C        ADC     A,H
1C4A 8D        ADC     A,L
1C4B 8E        ADC     A,(HL)
1C4C 90        SUB     B
1C4D 91        SUB     C
1C4E 92        SUB     D
1C4F 93        SUB     E
1C50 95        SUB     L
1C51 96        SUB     (HL)
1C52 97        SUB     A
1C53 98        SBC     A,B
1C54 9A        SBC     A,D
1C55 9B        SBC     A,E
1C56 9C        SBC     A,H
1C57 9D        SBC     A,L
1C58 9F        SBC     A,A
1C59 A0        AND     B
1C5A A1        AND     C
1C5B A2        AND     D
1C5C A4        AND     H
1C5D A5        AND     L
1C5E A6        AND     (HL)
1C5F A7        AND     A
1C60 A8        XOR     B
1C61 AA        XOR     D
1C62 AB        XOR     E
1C63 AC        XOR     H
1C64 AD        XOR     L
1C65 AE        XOR     (HL)
1C66 AF        XOR     A
1C67 B1        OR      C
1C68 B2        OR      D
1C69 B3        OR      E
1C6A B4        OR      H
1C6B B5        OR      L
1C6C B6        OR      (HL)
1C6D B7        OR      A
1C6E B8        CP      B
1C6F B9        CP      C
1C70 BA        CP      D
1C71 BC        CP      H
1C72 BD        CP      L
1C73 BE        CP      (HL)
1C74 BF        CP      A
1C75 C0        RET     NZ

1C76 C1        POP     BC
1C77 C2C3C4    JP      NZ,&C4C3
1C7A C5        PUSH    BC
1C7B C6C7      ADD     A,&C7
1C7D C8        RET     Z

1C7E C9        RET     

1C7F CACBCC    JP      Z,&CCCB
1C82 CDCECF    CALL    &CFCE
1C85 CF        RST     &08
1C86 D0        RET     NC

1C87 D1        POP     DE
1C88 D2D3D4    JP      NC,&D4D3
1C8B D5        PUSH    DE
1C8C D6D7      SUB     &D7
1C8E D7        RST     &10
1C8F D8        RET     C

1C90 D9        EXX     
1C91 DADBDC    JP      C,&DCDB
1C94 DCDDDE    CALL    C,&DEDD
1C97 DF        RST     &18
1C98 E0        RET     PO

1C99 E0        RET     PO

1C9A E1        POP     HL
1C9B E2E3E3    JP      PO,&E3E3
1C9E E4E5E5    CALL    PO,&E5E5
1CA1 E6E7      AND     &E7
1CA3 E7        RST     &20
1CA4 E8        RET     PE

1CA5 E9        JP      (HL)
1CA6 E9        JP      (HL)
1CA7 EAEBEB    JP      PE,&EBEB
1CAA ECEDED    CALL    PE,&EDED
1CAD EEEE      XOR     &EE
1CAF EF        RST     &28
1CB0 EF        RST     &28
1CB1 F0        RET     P

1CB2 F1        POP     AF
1CB3 F1        POP     AF
1CB4 F2F2F3    JP      P,&F3F2
1CB7 F3        DI      
1CB8 F4F4F5    CALL    P,&F5F4
1CBB F5        PUSH    AF
1CBC F5        PUSH    AF
1CBD F6F6      OR      &F6
1CBF F7        RST     &30
1CC0 F7        RST     &30
1CC1 F8        RET     M

1CC2 F8        RET     M

1CC3 F8        RET     M

1CC4 F9        LD      SP,HL
1CC5 F9        LD      SP,HL
1CC6 F9        LD      SP,HL
1CC7 FAFAFA    JP      M,&FAFA
1CCA FB        EI      
1CCB FB        EI      
1CCC FB        EI      
1CCD FCFCFC    CALL    M,&FCFC
1CD0 FCFDFD    CALL    M,&FDFD
1CD3 FDFDFEFE  CP      &FE
1CD7 FEFE      CP      &FE
1CD9 FEFF      CP      &FF
1CDB FF        RST     &38
1CDC FF        RST     &38
1CDD FF        RST     &38
1CDE FF        RST     &38
1CDF FF        RST     &38
1CE0 FF        RST     &38
1CE1 00        NOP     
1CE2 00        NOP     
1CE3 00        NOP     
1CE4 00        NOP     
1CE5 00        NOP     
1CE6 00        NOP     
1CE7 00        NOP     
1CE8 00        NOP     
1CE9 00        NOP     
1CEA 00        NOP     
1CEB AF        XOR     A
1CEC 1802      JR      &1CF0            ; (+&02)
1CEE 3E01      LD      A,&01
1CF0 3255FB    LD      (&FB55),A
1CF3 3E0A      LD      A,&0A
1CF5 3257FB    LD      (&FB57),A
1CF8 CDAD1D    CALL    &1DAD
1CFB D323      OUT     (&23),A
1CFD CDD71D    CALL    &1DD7
1D00 C28A1D    JP      NZ,&1D8A
1D03 CD821E    CALL    &1E82
1D06 7E        LD      A,(HL)
1D07 D319      OUT     (&19),A
1D09 3A51FB    LD      A,(&FB51)
1D0C BE        CP      (HL)
1D0D 2813      JR      Z,&1D22          ; (+&13)
1D0F 77        LD      (HL),A
1D10 D31B      OUT     (&1B),A
1D12 3E10      LD      A,&10
1D14 CD151E    CALL    &1E15
1D17 CC2E1E    CALL    Z,&1E2E
1D1A 11B9A0    LD      DE,&A0B9
1D1D CC491E    CALL    Z,&1E49
1D20 2068      JR      NZ,&1D8A         ; (+&68)
1D22 3A52FB    LD      A,(&FB52)
1D25 D31A      OUT     (&1A),A
1D27 3A44FB    LD      A,(&FB44)
1D2A 57        LD      D,A
1D2B 0E1B      LD      C,&1B
1D2D 2A53FB    LD      HL,(&FB53)
1D30 3A55FB    LD      A,(&FB55)
1D33 B7        OR      A
1D34 F3        DI      
1D35 2026      JR      NZ,&1D5D         ; (+&26)
1D37 3E80      LD      A,&80
1D39 0600      LD      B,&00
1D3B CD1B1E    CALL    &1E1B
1D3E 2014      JR      NZ,&1D54         ; (+&14)
1D40 DB18      IN      A,(&18)
1D42 0F        RRCA    
1D43 300C      JR      NC,&1D51         ; (+&0c)
1D45 0F        RRCA    
1D46 30F8      JR      NC,&1D40         ; (-&08)
1D48 EDA2      INI     
1D4A C2401D    JP      NZ,&1D40
1D4D 15        DEC     D
1D4E C2401D    JP      NZ,&1D40
1D51 CD2E1E    CALL    &1E2E
1D54 FB        EI      
1D55 2033      JR      NZ,&1D8A         ; (+&33)
1D57 DB18      IN      A,(&18)
1D59 E6BF      AND     &BF
1D5B 182A      JR      &1D87            ; (+&2a)
1D5D 3EA0      LD      A,&A0
1D5F 0600      LD      B,&00
1D61 CD1B1E    CALL    &1E1B
1D64 201C      JR      NZ,&1D82         ; (+&1c)
1D66 CD14FB    CALL    &FB14
1D69 5F        LD      E,A
1D6A DB18      IN      A,(&18)
1D6C 0F        RRCA    
1D6D 3010      JR      NC,&1D7F         ; (+&10)
1D6F 0F        RRCA    
1D70 30F8      JR      NC,&1D6A         ; (-&08)
1D72 ED59      OUT     (C),E
1D74 23        INC     HL
1D75 CD14FB    CALL    &FB14
1D78 5F        LD      E,A
1D79 10EF      DJNZ    &1D6A            ; (-&11)
1D7B 15        DEC     D
1D7C C26A1D    JP      NZ,&1D6A
1D7F CD2E1E    CALL    &1E2E
1D82 FB        EI      
1D83 2005      JR      NZ,&1D8A         ; (+&05)
1D85 DB18      IN      A,(&18)
1D87 FE80      CP      &80
1D89 C8        RET     Z

1D8A CDF71D    CALL    &1DF7
1D8D 2157FB    LD      HL,&FB57
1D90 FE04      CP      &04
1D92 3004      JR      NC,&1D98         ; (+&04)
1D94 FE02      CP      &02
1D96 2002      JR      NZ,&1D9A         ; (+&02)
1D98 3601      LD      (HL),&01
1D9A 3ED0      LD      A,&D0
1D9C D318      OUT     (&18),A
1D9E 35        DEC     (HL)
1D9F 2805      JR      Z,&1DA6          ; (+&05)
1DA1 CF        RST     &08
1DA2 AF        XOR     A
1DA3 CAF81C    JP      Z,&1CF8
1DA6 CF        RST     &08
1DA7 BD        CP      L
1DA8 3A56FB    LD      A,(&FB56)
1DAB B7        OR      A
1DAC C9        RET     

1DAD E5        PUSH    HL
1DAE CDCC1D    CALL    &1DCC
1DB1 6E        LD      L,(HL)
1DB2 3A52FB    LD      A,(&FB52)
1DB5 D60A      SUB     &0A
1DB7 3802      JR      C,&1DBB          ; (+&02)
1DB9 CBE5      SET     4,L
1DBB 7D        LD      A,L
1DBC E1        POP     HL
1DBD C9        RET     

1DBE E5        PUSH    HL
1DBF CDCC1D    CALL    &1DCC
1DC2 3AB1FB    LD      A,(&FBB1)
1DC5 A6        AND     (HL)
1DC6 E1        POP     HL
1DC7 3E0A      LD      A,&0A
1DC9 C8        RET     Z

1DCA 87        ADD     A,A
1DCB C9        RET     

1DCC 3A50FB    LD      A,(&FB50)
1DCF 213400    LD      HL,&0034
1DD2 E603      AND     &03
1DD4 85        ADD     A,L
1DD5 6F        LD      L,A
1DD6 C9        RET     

1DD7 AF        XOR     A
1DD8 DB18      IN      A,(&18)
1DDA 17        RLA     
1DDB D8        RET     C

1DDC 3E04      LD      A,&04
1DDE CD151E    CALL    &1E15
1DE1 CC581E    CALL    Z,&1E58
1DE4 CC2E1E    CALL    Z,&1E2E
1DE7 11BDA4    LD      DE,&A4BD
1DEA CC491E    CALL    Z,&1E49
1DED 2008      JR      NZ,&1DF7         ; (+&08)
1DEF CD821E    CALL    &1E82
1DF2 AF        XOR     A
1DF3 D319      OUT     (&19),A
1DF5 77        LD      (HL),A
1DF6 C9        RET     

1DF7 D5        PUSH    DE
1DF8 FEFE      CP      &FE
1DFA 1E05      LD      E,&05
1DFC 2810      JR      Z,&1E0E          ; (+&10)
1DFE 1D        DEC     E
1DFF FEFF      CP      &FF
1E01 280B      JR      Z,&1E0E          ; (+&0b)
1E03 1D        DEC     E
1E04 CB67      BIT     4,A
1E06 2006      JR      NZ,&1E0E         ; (+&06)
1E08 1D        DEC     E
1E09 CB77      BIT     6,A
1E0B 2001      JR      NZ,&1E0E         ; (+&01)
1E0D 1D        DEC     E
1E0E 7B        LD      A,E
1E0F D1        POP     DE
1E10 3256FB    LD      (&FB56),A
1E13 B7        OR      A
1E14 C9        RET     

1E15 E5        PUSH    HL
1E16 21B0FB    LD      HL,&FBB0
1E19 B6        OR      (HL)
1E1A E1        POP     HL
1E1B D318      OUT     (&18),A
1E1D C5        PUSH    BC
1E1E 0680      LD      B,&80
1E20 DB18      IN      A,(&18)
1E22 0F        RRCA    
1E23 3806      JR      C,&1E2B          ; (+&06)
1E25 10F9      DJNZ    &1E20            ; (-&07)
1E27 C1        POP     BC
1E28 AF        XOR     A
1E29 3D        DEC     A
1E2A C9        RET     

1E2B C1        POP     BC
1E2C AF        XOR     A
1E2D C9        RET     

1E2E C5        PUSH    BC
1E2F D5        PUSH    DE
1E30 1603      LD      D,&03
1E32 010000    LD      BC,&0000
1E35 DB18      IN      A,(&18)
1E37 0F        RRCA    
1E38 300B      JR      NC,&1E45         ; (+&0b)
1E3A 0B        DEC     BC
1E3B 78        LD      A,B
1E3C B1        OR      C
1E3D 20F6      JR      NZ,&1E35         ; (-&0a)
1E3F 15        DEC     D
1E40 20F3      JR      NZ,&1E35         ; (-&0d)
1E42 D1        POP     DE
1E43 18E2      JR      &1E27            ; (-&1e)
1E45 D1        POP     DE
1E46 C1        POP     BC
1E47 AF        XOR     A
1E48 C9        RET     

1E49 D5        PUSH    DE
1E4A 110011    LD      DE,&1100
1E4D 1B        DEC     DE
1E4E 7A        LD      A,D
1E4F B3        OR      E
1E50 20FB      JR      NZ,&1E4D         ; (-&05)
1E52 D1        POP     DE
1E53 DB18      IN      A,(&18)
1E55 A3        AND     E
1E56 BA        CP      D
1E57 C9        RET     

1E58 C5        PUSH    BC
1E59 010000    LD      BC,&0000
1E5C 0B        DEC     BC
1E5D 78        LD      A,B
1E5E B1        OR      C
1E5F 2808      JR      Z,&1E69          ; (+&08)
1E61 DB18      IN      A,(&18)
1E63 CB4F      BIT     1,A
1E65 28F5      JR      Z,&1E5C          ; (-&0b)
1E67 18C2      JR      &1E2B            ; (-&3e)
1E69 DB18      IN      A,(&18)
1E6B 17        RLA     
1E6C 17        RLA     
1E6D 3F        CCF     
1E6E 9F        SBC     A,A
1E6F 3D        DEC     A
1E70 C1        POP     BC
1E71 C9        RET     

1E72 AF        XOR     A
1E73 D323      OUT     (&23),A
1E75 C5        PUSH    BC
1E76 0620      LD      B,&20
1E78 D300      OUT     (&00),A
1E7A E3        EX      (SP),HL
1E7B E3        EX      (SP),HL
1E7C 10FA      DJNZ    &1E78            ; (-&06)
1E7E CF        RST     &08
1E7F C0        RET     NZ

1E80 C1        POP     BC
1E81 C9        RET     

1E82 3A50FB    LD      A,(&FB50)
1E85 2158FB    LD      HL,&FB58
1E88 85        ADD     A,L
1E89 6F        LD      L,A
1E8A D0        RET     NC

1E8B 24        INC     H
1E8C C9        RET     

1E8D 210000    LD      HL,&0000
1E90 79        LD      A,C
1E91 327DFB    LD      (&FB7D),A
1E94 FE04      CP      &04
1E96 D0        RET     NC

1E97 6F        LD      L,A
1E98 ED4BB3FB  LD      BC,(&FBB3)
1E9C 29        ADD     HL,HL
1E9D 29        ADD     HL,HL
1E9E 29        ADD     HL,HL
1E9F 29        ADD     HL,HL
1EA0 09        ADD     HL,BC
1EA1 C9        RET     

1EA2 79        LD      A,C
1EA3 327EFB    LD      (&FB7E),A
1EA6 C9        RET     

1EA7 79        LD      A,C
1EA8 327FFB    LD      (&FB7F),A
1EAB C9        RET     

1EAC ED4380FB  LD      (&FB80),BC
1EB0 C9        RET     

1EB1 2100FE    LD      HL,&FE00
1EB4 2253FB    LD      (&FB53),HL
1EB7 AF        XOR     A
1EB8 3287FB    LD      (&FB87),A
1EBB 3C        INC     A
1EBC 328AFB    LD      (&FB8A),A
1EBF 3C        INC     A
1EC0 328BFB    LD      (&FB8B),A
1EC3 CDB41F    CALL    &1FB4
1EC6 79        LD      A,C
1EC7 FE01      CP      &01
1EC9 2866      JR      Z,&1F31          ; (+&66)
1ECB 1850      JR      &1F1D            ; (+&50)
1ECD 2100FE    LD      HL,&FE00
1ED0 2253FB    LD      (&FB53),HL
1ED3 AF        XOR     A
1ED4 CDB41F    CALL    &1FB4
1ED7 79        LD      A,C
1ED8 328BFB    LD      (&FB8B),A
1EDB FE02      CP      &02
1EDD 2011      JR      NZ,&1EF0         ; (+&11)
1EDF 3E10      LD      A,&10
1EE1 3287FB    LD      (&FB87),A
1EE4 2A7DFB    LD      HL,(&FB7D)
1EE7 2283FB    LD      (&FB83),HL
1EEA 3A7FFB    LD      A,(&FB7F)
1EED 3285FB    LD      (&FB85),A
1EF0 3A87FB    LD      A,(&FB87)
1EF3 B7        OR      A
1EF4 281F      JR      Z,&1F15          ; (+&1f)
1EF6 3D        DEC     A
1EF7 3287FB    LD      (&FB87),A
1EFA 3A7FFB    LD      A,(&FB7F)
1EFD 2183FB    LD      HL,&FB83
1F00 CDC21F    CALL    &1FC2
1F03 2010      JR      NZ,&1F15         ; (+&10)
1F05 34        INC     (HL)
1F06 CDBE1D    CALL    &1DBE
1F09 87        ADD     A,A
1F0A 87        ADD     A,A
1F0B BE        CP      (HL)
1F0C 2004      JR      NZ,&1F12         ; (+&04)
1F0E 3600      LD      (HL),&00
1F10 2B        DEC     HL
1F11 34        INC     (HL)
1F12 AF        XOR     A
1F13 1805      JR      &1F1A            ; (+&05)
1F15 AF        XOR     A
1F16 3287FB    LD      (&FB87),A
1F19 3C        INC     A
1F1A 328AFB    LD      (&FB8A),A
1F1D 2186FB    LD      HL,&FB86
1F20 7E        LD      A,(HL)
1F21 3601      LD      (HL),&01
1F23 B7        OR      A
1F24 2815      JR      Z,&1F3B          ; (+&15)
1F26 3A82FB    LD      A,(&FB82)
1F29 2150FB    LD      HL,&FB50
1F2C CDC21F    CALL    &1FC2
1F2F 2833      JR      Z,&1F64          ; (+&33)
1F31 3A88FB    LD      A,(&FB88)
1F34 B7        OR      A
1F35 2804      JR      Z,&1F3B          ; (+&04)
1F37 CF        RST     &08
1F38 A3        AND     E
1F39 206F      JR      NZ,&1FAA         ; (+&6f)
1F3B 2A7DFB    LD      HL,(&FB7D)
1F3E 3A50FB    LD      A,(&FB50)
1F41 BD        CP      L
1F42 2250FB    LD      (&FB50),HL
1F45 2809      JR      Z,&1F50          ; (+&09)
1F47 CDAD1D    CALL    &1DAD
1F4A D323      OUT     (&23),A
1F4C CF        RST     &08
1F4D AF        XOR     A
1F4E 205A      JR      NZ,&1FAA         ; (+&5a)
1F50 3A82FB    LD      A,(&FB82)
1F53 3252FB    LD      (&FB52),A
1F56 3A8AFB    LD      A,(&FB8A)
1F59 B7        OR      A
1F5A 2804      JR      Z,&1F60          ; (+&04)
1F5C CF        RST     &08
1F5D A2        AND     D
1F5E 204A      JR      NZ,&1FAA         ; (+&4a)
1F60 AF        XOR     A
1F61 3288FB    LD      (&FB88),A
1F64 3A7FFB    LD      A,(&FB7F)
1F67 0E00      LD      C,&00
1F69 E603      AND     &03
1F6B 1F        RRA     
1F6C CB19      RR      C
1F6E 47        LD      B,A
1F6F 2100FE    LD      HL,&FE00
1F72 09        ADD     HL,BC
1F73 018000    LD      BC,&0080
1F76 ED5B80FB  LD      DE,(&FB80)
1F7A 3A89FB    LD      A,(&FB89)
1F7D B7        OR      A
1F7E 2006      JR      NZ,&1F86         ; (+&06)
1F80 EB        EX      DE,HL
1F81 3E01      LD      A,&01
1F83 3288FB    LD      (&FB88),A
1F86 CF        RST     &08
1F87 B2        OR      D
1F88 3A8BFB    LD      A,(&FB8B)
1F8B FE01      CP      &01
1F8D 2008      JR      NZ,&1F97         ; (+&08)
1F8F AF        XOR     A
1F90 3288FB    LD      (&FB88),A
1F93 CF        RST     &08
1F94 A3        AND     E
1F95 2013      JR      NZ,&1FAA         ; (+&13)
1F97 AF        XOR     A
1F98 C9        RET     

1F99 3A88FB    LD      A,(&FB88)
1F9C B7        OR      A
1F9D C8        RET     Z

1F9E AF        XOR     A
1F9F 3288FB    LD      (&FB88),A
1FA2 2100FE    LD      HL,&FE00
1FA5 2253FB    LD      (&FB53),HL
1FA8 CF        RST     &08
1FA9 A3        AND     E
1FAA F5        PUSH    AF
1FAB AF        XOR     A
1FAC 3286FB    LD      (&FB86),A
1FAF 3287FB    LD      (&FB87),A
1FB2 F1        POP     AF
1FB3 C9        RET     

1FB4 3289FB    LD      (&FB89),A
1FB7 3A7FFB    LD      A,(&FB7F)
1FBA CB3F      SRL     A
1FBC CB3F      SRL     A
1FBE 3282FB    LD      (&FB82),A
1FC1 C9        RET     

1FC2 4F        LD      C,A
1FC3 3A7DFB    LD      A,(&FB7D)
1FC6 BE        CP      (HL)
1FC7 C0        RET     NZ

1FC8 3A7EFB    LD      A,(&FB7E)
1FCB 23        INC     HL
1FCC BE        CP      (HL)
1FCD C0        RET     NZ

1FCE 79        LD      A,C
1FCF 23        INC     HL
1FD0 BE        CP      (HL)
1FD1 C9        RET     

1FD2 FF        RST     &38
1FD3 FF        RST     &38
1FD4 FF        RST     &38
1FD5 FF        RST     &38
1FD6 FF        RST     &38
1FD7 FF        RST     &38
1FD8 FF        RST     &38
1FD9 FF        RST     &38
1FDA FF        RST     &38
1FDB FF        RST     &38
1FDC FF        RST     &38
1FDD FF        RST     &38
1FDE FF        RST     &38
1FDF FF        RST     &38
1FE0 FF        RST     &38
1FE1 FF        RST     &38
1FE2 FF        RST     &38
1FE3 FF        RST     &38
1FE4 FF        RST     &38
1FE5 FF        RST     &38
1FE6 FF        RST     &38
1FE7 FF        RST     &38
1FE8 FF        RST     &38
1FE9 FF        RST     &38
1FEA FF        RST     &38
1FEB FF        RST     &38
1FEC FF        RST     &38
1FED FF        RST     &38
1FEE FF        RST     &38
1FEF FF        RST     &38
1FF0 FF        RST     &38
1FF1 FF        RST     &38
1FF2 FF        RST     &38
1FF3 FF        RST     &38
1FF4 FF        RST     &38
1FF5 FF        RST     &38
1FF6 FF        RST     &38
1FF7 FF        RST     &38
1FF8 FF        RST     &38
1FF9 FF        RST     &38
1FFA FF        RST     &38
1FFB FF        RST     &38
1FFC FF        RST     &38
1FFD FF        RST     &38
1FFE FF        RST     &38
1FFF FF        RST     &38
2000 210000    LD      HL,&0000
