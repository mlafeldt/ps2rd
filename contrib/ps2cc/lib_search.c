/****************************************************************************
Search Library

This is where the main search loop runs, as well as supplemental search
functions. A few specific globals are in use (I think just RamInfo), and 1 API
(progress bar). If updating/adding options at any point, please make an effort
not to utilize for any globals or APIs that aren't already in use.

Each comparison type actually compares to opposite. You start with every
address as a possibly. Then find the bad results and turn them OFF in the list.
SetBitflag may seem overly complicated, but you should be able to add your own
options without really understanding it. Just do what I did. Every call to
SetBitFlag in the main search loop is settings addreses OFF as results.
*****************************************************************************/
#include "ps2cc.h"


/****************************************************************************
CodeSearch - main search loop
*****************************************************************************/
int CodeSearch(CODE_SEARCH_VARS Search)
{
    u32 address;
    u64 NewValue;
    u64 OldValue;
    RamInfo.NewResultsInfo.ResCount = 0;
    if (Search.TypeEx & EXCS_SIGNED) {
        int i;
        for (i = 0; i < 10; i++) { Search.Values[i] = SignExtend64(Search.Values[i], Search.Size); }
    }
    for (address = 0; address < RamInfo.NewResultsInfo.DumpSize; address += Search.Size) {
        if(!(address % 0x100000)) { UpdateProgressBar(PBM_STEPIT, 0, 0); }
        if (!(GetBitFlag(RamInfo.Results, address/Search.Size))) { continue; }
        GetSearchValues(&NewValue, &OldValue, address, Search.Size, LITTLE_ENDIAN_SYS);
        if (!CodeSearchEx(address, NewValue, OldValue, Search)) { continue; }
        if (Search.TypeEx & EXCS_SIGNED) {
            NewValue = SignExtend64(NewValue, Search.Size);
            OldValue = SignExtend64(OldValue, Search.Size);
        }
        switch(Search.Type)
        {
            case SEARCH_KNOWN:
            {
                if (NewValue != Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_KNOWN_WILD:
            {
                if ((NewValue & Search.Values[1]) != Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_GREATER:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue <= (s64)OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue <= OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_GREATER_BY:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue != ((s64)OldValue + Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue != (OldValue + Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_GREATER_LEAST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue < ((s64)OldValue + Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue < (OldValue + Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_GREATER_MOST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue > ((s64)OldValue + Search.Values[0])) || ((s64)NewValue <= (s64)OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue > (OldValue + Search.Values[0])) || (NewValue <= OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_LESS:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue >= (s64)OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue >= OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_LESS_BY:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue != ((s64)OldValue - Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue != (OldValue - Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_LESS_LEAST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if ((s64)NewValue > ((s64)OldValue - Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if (NewValue > (OldValue - Search.Values[0])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_LESS_MOST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue < ((s64)OldValue - Search.Values[0])) || ((s64)NewValue >= (s64)OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue < (OldValue - Search.Values[0])) || (NewValue >= OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_EQUAL:
            {
                if (NewValue != OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_EQUAL_NUM_BITS:
            {
                if (BitCount(NewValue) != Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL:
            {
                if (NewValue == OldValue) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_TO:
            {
                if (NewValue == Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_BY:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue != ((s64)OldValue - Search.Values[0])) && ((s64)NewValue != ((s64)OldValue + Search.Values[0]))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue != (OldValue - Search.Values[0])) && (NewValue != (OldValue + Search.Values[0]))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_LEAST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue > ((s64)OldValue - Search.Values[0])) && ((s64)NewValue < ((s64)OldValue + Search.Values[0]))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue > (OldValue - Search.Values[0])) && (NewValue < (OldValue + Search.Values[0]))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_MOST:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue < ((s64)OldValue - Search.Values[0])) || ((s64)NewValue > ((s64)OldValue + Search.Values[0])) || (NewValue == OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue < (OldValue - Search.Values[0])) || (NewValue > (OldValue + Search.Values[0])) || (NewValue == OldValue)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_TO_BITS:
            {
                if (BitCount(NewValue) == Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NEQUAL_BY_BITS:
            {
               if (BitCount(NewValue ^ OldValue) != Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_IN_RANGE:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue <= Search.Values[0]) || ((s64)NewValue > Search.Values[1])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue <= Search.Values[0]) || (NewValue > Search.Values[1])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_NOT_RANGE:
            {
                if (Search.TypeEx & EXCS_SIGNED) {
                    if (((s64)NewValue >= Search.Values[0]) && ((s64)NewValue <= Search.Values[1])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
                } else if ((NewValue >= Search.Values[0]) && (NewValue <= Search.Values[1])) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_BITS_ANY:
            {
                if ((NewValue & Search.Values[0]) == 0) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
            case SEARCH_BITS_ALL:
            {
                if ((NewValue & Search.Values[0]) != Search.Values[0]) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); }
            } break;
        }
        if (GetBitFlag(RamInfo.Results, address/Search.Size)) { RamInfo.NewResultsInfo.ResCount++; }
    }
    if (Search.TypeEx & (EXCS_EXCLUDE_CONSEC|EXCS_INCLUDE_CONSEC|EXCS_EXCLUDE_CONSEC_MATCH_VALUES|EXCS_INCLUDE_CONSEC_MATCH_VALUES)) { FilterResultsEx(Search); }
    return 1;
}

/****************************************************************************
Get Search Values -- RamInfo.NewRAM or RamInfo.NewFile must be loaded before
calling this. It grabs any size value from the address (index) and loads them
into NewVal and OldVal via pointer.
*****************************************************************************/
u64 GetSearchValues(u64 *NewVal, u64 *OldVal, int index, int size, int endian)
{
    int i;
//    u64 tmpvalue = 0;
    *NewVal = 0;
    if(RamInfo.OldRAM) {  *OldVal = 0; }
    if (RamInfo.Access == SEARCH_ACCESS_ARRAY) {
        switch (endian)
        {
            case LITTLE_ENDIAN_SYS:
            {
                switch (size)
                {
                    case 1:
                    {
                        *NewVal = RamInfo.NewRAM[index];
                        if(RamInfo.OldRAM) { *OldVal = RamInfo.OldRAM[index]; }
                    } return 1;
                    case 2:
                    {
                        *NewVal = *(u16*)(&RamInfo.NewRAM[index]);
                        if(RamInfo.OldRAM) { *OldVal = *(u16*)(&RamInfo.OldRAM[index]); }
                    } return 1;
                    case 4:
                    {
                        *NewVal = *(u32*)(&RamInfo.NewRAM[index]);
                        if(RamInfo.OldRAM) { *OldVal = *(u32*)(&RamInfo.OldRAM[index]); }
                    } return 1;
                    case 8:
                    {
                        *NewVal = MAKEU64(*(u32*)(&RamInfo.NewRAM[index+4]),*(u32*)(&RamInfo.NewRAM[index]));
                        if(RamInfo.OldRAM) { *OldVal = MAKEU64(*(u32*)(&RamInfo.OldRAM[index+4]),*(u32*)(&RamInfo.OldRAM[index])); }
                    } return 1;
                }
            } break;
        }
    } else {
        switch (endian)
        {
            case LITTLE_ENDIAN_SYS:
            {
                fseek(RamInfo.NewFile,index,SEEK_SET);
                if (RamInfo.OldFile) { fseek(RamInfo.OldFile,index,SEEK_SET); }
                switch (size)
                {
                    case 1:
                    {
                        *NewVal = getc(RamInfo.NewFile);
                        if (RamInfo.OldFile) { *OldVal = getc(RamInfo.OldFile); }
                    } return 1;
                    case 2:
                    {
                        *NewVal = getc(RamInfo.NewFile) | (getc(RamInfo.NewFile)<<8);
                        if (RamInfo.OldFile) { *OldVal = getc(RamInfo.OldFile) | (getc(RamInfo.OldFile)<<8); }
                    } return 1;
                    case 4:
                    {
                        *NewVal = getw(RamInfo.NewFile);
                        if (RamInfo.OldFile) { *OldVal = getw(RamInfo.OldFile); }
                    } return 1;
                    case 8:
                    {
                        *NewVal = ((u64)getw(RamInfo.NewFile)<<32) | getw(RamInfo.NewFile);
                        if (RamInfo.OldFile) { *OldVal = ((u64)getw(RamInfo.OldFile)<<32) | getw(RamInfo.OldFile); }
                    } return 1;
                }
            } break;
        }
    }
    return 0;
}

/****************************************************************************
Set ExSearch Values based on ExSearch type
*****************************************************************************/
int SetExValues(CODE_SEARCH_VARS *SearchInfo, u64 exType, u64 exValue1, u64 exValue2)
{
    int bitnumber = 0;
    switch (exType)
    {
        case EXCS_IGNORE_VALUE: { bitnumber = 4; } break;
        case EXCS_IGNORE_IN_RANGE: { bitnumber = 5; } break;
        case EXCS_IGNORE_NOT_RANGE: { bitnumber = 6; } break;
        case EXCS_IGNORE_BYTE_VALUE: { bitnumber = 8; } break;
        case EXCS_IGNORE_SHORT_VALUE: { bitnumber = 9; } break;
        case EXCS_IGNORE_WORD_VALUE: { bitnumber = 10; } break;
        case EXCS_IGNORE_DWORD_VALUE: { bitnumber = 11; } break;
        case EXCS_IGNORE_BYTE_RANGE: { bitnumber = 12; } break;
        case EXCS_IGNORE_SHORT_RANGE: { bitnumber = 13; } break;
        case EXCS_IGNORE_WORD_RANGE: { bitnumber = 14; } break;
        case EXCS_IGNORE_DWORD_RANGE: { bitnumber = 15; } break;
        case EXCS_EXCLUDE_CONSEC: case EXCS_EXCLUDE_CONSEC_MATCH_VALUES:
        case EXCS_INCLUDE_CONSEC: case EXCS_INCLUDE_CONSEC_MATCH_VALUES: { bitnumber = 16; } break;
        case EXCS_INCLUDE_ADDRESS_RANGE: { bitnumber = 17; } break;
    }
    SearchInfo->ValuesEx1[bitnumber] = exValue1;
    SearchInfo->ValuesEx2[bitnumber] = exValue2;
    return 0;
}
/****************************************************************************
Get ExSearch Value based on ExSearch type
*****************************************************************************/
u64 GetExSearchValue(u64 *exValues, u64 exType)
{
    switch (exType)
    {
        case EXCS_IGNORE_VALUE: { return exValues[4]; }
        case EXCS_IGNORE_IN_RANGE: { return exValues[5]; }
        case EXCS_IGNORE_NOT_RANGE: { return exValues[6]; }
        case EXCS_IGNORE_BYTE_VALUE: { return exValues[8]; }
        case EXCS_IGNORE_SHORT_VALUE: { return exValues[9]; }
        case EXCS_IGNORE_WORD_VALUE: { return exValues[10]; }
        case EXCS_IGNORE_DWORD_VALUE: { return exValues[11]; }
        case EXCS_IGNORE_BYTE_RANGE: { return exValues[12]; }
        case EXCS_IGNORE_SHORT_RANGE: { return exValues[13]; }
        case EXCS_IGNORE_WORD_RANGE: { return exValues[14]; }
        case EXCS_IGNORE_DWORD_RANGE: { return exValues[15]; }
        case EXCS_EXCLUDE_CONSEC: case EXCS_EXCLUDE_CONSEC_MATCH_VALUES:
        case EXCS_INCLUDE_CONSEC: case EXCS_INCLUDE_CONSEC_MATCH_VALUES: { return exValues[16]; }
        case EXCS_INCLUDE_ADDRESS_RANGE: { return exValues[17]; }
        default: return 0;
    }
}

/****************************************************************************
CodeSearchEx
*****************************************************************************/
int CodeSearchEx(u32 address, u64 NewValue, u64 OldValue, CODE_SEARCH_VARS Search)
{
    u64 tmpValue1, tmpValue2;
    int i;
    if (!(Search.TypeEx)) { return 1; }
    if (Search.TypeEx & EXCS_INCLUDE_ADDRESS_RANGE) {
        if (((address|RamInfo.NewResultsInfo.MapMemAddy) < GetExSearchValue(Search.ValuesEx1,EXCS_INCLUDE_ADDRESS_RANGE)) ||
            ((address|RamInfo.NewResultsInfo.MapMemAddy) > GetExSearchValue(Search.ValuesEx2,EXCS_INCLUDE_ADDRESS_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    }
    if ((Search.TypeEx & EXCS_IGNORE_0) && (!(NewValue))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    if ((Search.TypeEx & EXCS_IGNORE_FF) &&  (NewValue == (0xFFFFFFFFFFFFFFFFLL >> 8*(8-Search.Size)))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    if ((Search.TypeEx & EXCS_IGNORE_VALUE) && (NewValue == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_VALUE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    if ((Search.TypeEx & EXCS_IGNORE_IN_RANGE) && (NewValue >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_IN_RANGE)) &&
        (NewValue <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_IN_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    if ((Search.TypeEx & EXCS_IGNORE_NOT_RANGE) && ((NewValue <= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_NOT_RANGE)) ||
        (NewValue >= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_NOT_RANGE)))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    GetSearchValues(&tmpValue1, &tmpValue2, (address & ~7), 8, LITTLE_ENDIAN_SYS);

    if ((Search.TypeEx & EXCS_IGNORE_BYTE_VALUE)) {
        for (i = 0; i < Search.Size; i++) {
            if (ByteFromU64(NewValue, 7 - i) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_BYTE_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_SHORT_VALUE)) {
        switch(Search.Size)
        {
            case 1: case 2:
            {
                if (ShortFromU64(NewValue, (address & 7) / 2) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
            } break;
            case 4:
            {
                if ((ShortFromU64(NewValue, (address & 7) / 4) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_VALUE)) ||
                    (ShortFromU64(NewValue, ((address & 7) / 4) + 1)) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
            } break;
            case 8:
            {
                for (i = 0; i < 4; i++) {
                    if (ShortFromU64(NewValue, i) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
                }
            } break;
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_WORD_VALUE)) {
        if (WordFromU64(NewValue, (address & 7) / 4) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_WORD_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        if (Search.Size > 4) {
            if (WordFromU64(NewValue, ((address & 7) / 4)^1) == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_WORD_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_DWORD_VALUE)) {
        if (NewValue == GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_DWORD_VALUE)) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    }
    if ((Search.TypeEx & EXCS_IGNORE_BYTE_RANGE)) {
        for (i = 0; i < Search.Size; i++) {
            if ((ByteFromU64(NewValue, 7 - i) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_BYTE_RANGE)) &&
                (ByteFromU64(NewValue, 7 - i) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_BYTE_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_SHORT_RANGE)) {
        switch(Search.Size)
        {
            case 1: case 2:
            {
                if ((ShortFromU64(NewValue, (address & 7) / 2) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_RANGE)) &&
                    (ShortFromU64(NewValue, (address & 7) / 2) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_SHORT_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
            } break;
            case 4:
            {
                if (((ShortFromU64(NewValue, (address & 7) / 4) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_RANGE)) &&
                    (ShortFromU64(NewValue, (address & 7) / 4) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_SHORT_RANGE))) ||
                    (((ShortFromU64(NewValue, ((address & 7) / 4) + 1)) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_RANGE)) &&
                     (ShortFromU64(NewValue, ((address & 7) / 4) + 1)) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_SHORT_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
            } break;
            case 8:
            {
                for (i = 0; i < 4; i++) {
                    if ((ShortFromU64(NewValue, i) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_SHORT_RANGE)) &&
                        (ShortFromU64(NewValue, i) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_SHORT_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
                }
            } break;
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_WORD_RANGE)) {
        if ((WordFromU64(NewValue, (address & 7) / 4) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_WORD_RANGE)) &&
            (WordFromU64(NewValue, (address & 7) / 4) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_WORD_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        if (Search.Size > 4) {
            if ((WordFromU64(NewValue, ((address & 7) / 4)^1) >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_WORD_RANGE)) &&
                (WordFromU64(NewValue, ((address & 7) / 4)^1) <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_WORD_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
        }
    }
    if ((Search.TypeEx & EXCS_IGNORE_DWORD_RANGE)) {
        if ((NewValue >= GetExSearchValue(Search.ValuesEx1,EXCS_IGNORE_DWORD_RANGE)) &&
            (NewValue <= GetExSearchValue(Search.ValuesEx2,EXCS_IGNORE_DWORD_RANGE))) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    }

    if ((Search.TypeEx & EXCS_EXCLUDE_UPPER16) && (Search.Size == 2)) {
        if ((address % 4) == 0) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    }
    if ((Search.TypeEx & EXCS_EXCLUDE_LOWER16) && (Search.Size == 2)) {
        if (address % 4) { SetBitFlag(RamInfo.Results, address/Search.Size, 0); return 0; }
    }

    if ((Search.TypeEx & EXCS_OR_EQUAL) && (NewValue == OldValue)) { return 0; }
    return 1;
}
/****************************************************************************
FilterResultsEx - consec/matching values searchEx types
*****************************************************************************/
int FilterResultsEx(CODE_SEARCH_VARS Search)
{
    u32 address, i, matches;
    u64 NewValue, NewValue2, OldValue;
    for (address = 0; address < RamInfo.NewResultsInfo.DumpSize; address += Search.Size) {
        if(!(address % 0x100000)) { UpdateProgressBar(PBM_STEPIT, 0, 0); }
        if (!(GetBitFlag(RamInfo.Results, address/Search.Size))) { continue; }
        GetSearchValues(&NewValue, &OldValue, address, Search.Size, LITTLE_ENDIAN_SYS);
        matches = 1;
        for (i = address + Search.Size; i < RamInfo.NewResultsInfo.DumpSize; i += Search.Size)
        {
            if (!(GetBitFlag(RamInfo.Results, address/Search.Size))) { break; }
            GetSearchValues(&NewValue2, &OldValue, i, Search.Size, LITTLE_ENDIAN_SYS);
            if ((Search.TypeEx & (EXCS_EXCLUDE_CONSEC|EXCS_INCLUDE_CONSEC)) && (!(GetBitFlag(RamInfo.Results, i/Search.Size)))) { break; }
            if ((Search.TypeEx & (EXCS_EXCLUDE_CONSEC_MATCH_VALUES|EXCS_INCLUDE_CONSEC_MATCH_VALUES)) &&
                ((!(GetBitFlag(RamInfo.Results, i/Search.Size))) || (NewValue != NewValue2))) { break; }
            matches++;
        }
        if (((matches >= GetExSearchValue(Search.ValuesEx1,EXCS_EXCLUDE_CONSEC)) && (Search.TypeEx & (EXCS_EXCLUDE_CONSEC|EXCS_EXCLUDE_CONSEC_MATCH_VALUES))) ||
            ((matches <= GetExSearchValue(Search.ValuesEx1,EXCS_INCLUDE_CONSEC)) && (Search.TypeEx & (EXCS_INCLUDE_CONSEC|EXCS_INCLUDE_CONSEC_MATCH_VALUES)))){
            while (address < i) {
                SetBitFlag(RamInfo.Results, address/Search.Size, 0);
                RamInfo.NewResultsInfo.ResCount--;
                address++;
            }
            address -= Search.Size;
        } else { address = i; }
    }
    return 0;
}

