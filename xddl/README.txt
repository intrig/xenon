3GPP:
    http://en.wikipedia.org/wiki/3GPP

    UMTS overview:
        http://en.wikipedia.org/wiki/Universal_Mobile_Telecommunications_System

    W-CDMA: 
        http://en.wikipedia.org/wiki/W-CDMA_%28UMTS%29

    GSM/GPRS: 
        http://en.wikipedia.org/wiki/Enhanced_Data_Rates_for_GSM_Evolution

    LTE: 
        http://en.wikipedia.org/wiki/3GPP_Long_Term_Evolution

3GPP2:
    http://en.wikipedia.org/wiki/3rd_Generation_Partnership_Project_2

    cdma2000:
        http://en.wikipedia.org/wiki/CDMA2000
    
    EVDO: 
        http://en.wikipedia.org/wiki/Evdo

======================

rrc, rlc -- wcdma

nas-ies, common between wcdma and gsm

nas-rr-ies, nas-rr -- GSM Release 99


rrc.xddl is TS 25.331 : UMTS RRC Release 9
    channels 0 through 8
    
    SIB: broadcast data, defined in rrc

NAS - 24.008

Radio Resource layer 2 and RR Short are defined in RR -- 44.018


The other channels should not be channels
Channel 11 : 25.331

The rest are in RLC -- 25.322
Protocol Technologies and Directories

==================================================

cdma2000
-----------------------------
3GPP2/C.S0004-E: Signaling Link Access Control (LAC) 
3GPP2/C.S0005-E: Upper Layer (Layer 3) Signaling 
3GPP2/C.S0015-B: SMS Short Message Service (SMS) for Wideband Spread Spectrum Systems 
3GPP2/C.R1001-G: Administration of Parameter Value Assignments
3GPP2/C.S0057-C: Band Class Specification


EVDO
----------------------------
TODO: organize similar to cdma2000 Technology

3GPP2/C.S0024: EVDO (C.S0024-B-v3.0)

    These are unsupported at the moment:
    C.S0063: Multiflow (EMPA, MMPA, latest version)
    C.S0029: TAP

WCDMA and GSM
-----------------------------
TODO: These need to be organized by document similar to the above technologies:

3GPP2/umts/rrc and rlc           : WCDMA (what is the spec?)
3GPP2/umts/nas-rr-ies and nas-rr : GSM Release 99 (3GPP TS 04.18 version 8.18.0 Release 1999)
3GPP2/umts/nas-ies               : common between the two

LTE
-----------------------------

3GPP2/TS-24.301-8: LTE NAS 
3GPP2/TS-36.331-8: LTE RRC (ETSI TS 136 331)

------

Additional Notes:

Here is what I got by now.
 

For CDMA, IS-2000-0-2 series specs are used. They are C.S0001-0-2, C.S0002-0-2, C.S0003-0-2, C.S0004-0-2, C.S0005-0-2 and C.S0006-0-2, all with version 1.0.

 

For EVDO, the spec is C.S0024-B-v3.0, with additional messages:

 

All the Multiflow (EMPA, MMPA) are defined in the supplemental spec C.S0063 (latest version).

TAP messages are defined in the latest spec C.S0029.

 

For the message direction, I think the following should be OK.

 

        F_SYNC = 0, SYNC_CHANNEL - Downlink

        F_PCH = 1, PAGING_CHANNEL - Downlink

        F_FCH = 2, FORWARD_TRAFFIC_CHANNEL - Downlink

        R_ACH = 3, ACCESS_CHANNEL - uplink

        R_FCH = 4, REVERSE_TRAFFIC_CHANNEL - uplink









