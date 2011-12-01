#pragma once

	// Reserved by CPU
	SetInterrupt(0x00, reinterpret_cast<uint32_t>(IDTISR00));
	SetInterrupt(0x01, reinterpret_cast<uint32_t>(IDTISR01));
	SetInterrupt(0x02, reinterpret_cast<uint32_t>(IDTISR02));
	SetInterrupt(0x03, reinterpret_cast<uint32_t>(IDTISR03));
	SetInterrupt(0x04, reinterpret_cast<uint32_t>(IDTISR04));
	SetInterrupt(0x05, reinterpret_cast<uint32_t>(IDTISR05));
	SetInterrupt(0x06, reinterpret_cast<uint32_t>(IDTISR06));
	SetInterrupt(0x07, reinterpret_cast<uint32_t>(IDTISR07));
	SetInterrupt(0x08, reinterpret_cast<uint32_t>(IDTISR08));
	SetInterrupt(0x09, reinterpret_cast<uint32_t>(IDTISR09));
	SetInterrupt(0x0A, reinterpret_cast<uint32_t>(IDTISR0A));
	SetInterrupt(0x0B, reinterpret_cast<uint32_t>(IDTISR0B));
	SetInterrupt(0x0C, reinterpret_cast<uint32_t>(IDTISR0C));
	SetInterrupt(0x0D, reinterpret_cast<uint32_t>(IDTISR0D));
	SetInterrupt(0x0E, reinterpret_cast<uint32_t>(IDTISR0E));
	SetInterrupt(0x0F, reinterpret_cast<uint32_t>(IDTISR0F));
	SetInterrupt(0x10, reinterpret_cast<uint32_t>(IDTISR10));
	SetInterrupt(0x11, reinterpret_cast<uint32_t>(IDTISR11));
	SetInterrupt(0x12, reinterpret_cast<uint32_t>(IDTISR12));
	SetInterrupt(0x13, reinterpret_cast<uint32_t>(IDTISR13));
	SetInterrupt(0x14, reinterpret_cast<uint32_t>(IDTISR14));
	SetInterrupt(0x15, reinterpret_cast<uint32_t>(IDTISR15));
	SetInterrupt(0x16, reinterpret_cast<uint32_t>(IDTISR16));
	SetInterrupt(0x17, reinterpret_cast<uint32_t>(IDTISR17));
	SetInterrupt(0x18, reinterpret_cast<uint32_t>(IDTISR18));
	SetInterrupt(0x19, reinterpret_cast<uint32_t>(IDTISR19));
	SetInterrupt(0x1A, reinterpret_cast<uint32_t>(IDTISR1A));
	SetInterrupt(0x1B, reinterpret_cast<uint32_t>(IDTISR1B));
	SetInterrupt(0x1C, reinterpret_cast<uint32_t>(IDTISR1C));
	SetInterrupt(0x1D, reinterpret_cast<uint32_t>(IDTISR1D));
	SetInterrupt(0x1E, reinterpret_cast<uint32_t>(IDTISR1E));
	SetInterrupt(0x1F, reinterpret_cast<uint32_t>(IDTISR1F));
	// Reserved for IRQ
	SetInterrupt(0x20, reinterpret_cast<uint32_t>(IDTIRQ00));
	SetInterrupt(0x21, reinterpret_cast<uint32_t>(IDTIRQ01));
	SetInterrupt(0x22, reinterpret_cast<uint32_t>(IDTIRQ02));
	SetInterrupt(0x23, reinterpret_cast<uint32_t>(IDTIRQ03));
	SetInterrupt(0x24, reinterpret_cast<uint32_t>(IDTIRQ04));
	SetInterrupt(0x25, reinterpret_cast<uint32_t>(IDTIRQ05));
	SetInterrupt(0x26, reinterpret_cast<uint32_t>(IDTIRQ06));
	SetInterrupt(0x27, reinterpret_cast<uint32_t>(IDTIRQ07));
	SetInterrupt(0x28, reinterpret_cast<uint32_t>(IDTIRQ08));
	SetInterrupt(0x29, reinterpret_cast<uint32_t>(IDTIRQ09));
	SetInterrupt(0x2A, reinterpret_cast<uint32_t>(IDTIRQ0A));
	SetInterrupt(0x2B, reinterpret_cast<uint32_t>(IDTIRQ0B));
	SetInterrupt(0x2C, reinterpret_cast<uint32_t>(IDTIRQ0C));
	SetInterrupt(0x2D, reinterpret_cast<uint32_t>(IDTIRQ0D));
	SetInterrupt(0x2E, reinterpret_cast<uint32_t>(IDTIRQ0E));
	SetInterrupt(0x2F, reinterpret_cast<uint32_t>(IDTIRQ0F));
	// Usable
	SetInterrupt(0x30, reinterpret_cast<uint32_t>(IDTISR30));
	SetInterrupt(0x31, reinterpret_cast<uint32_t>(IDTISR31));
	SetInterrupt(0x32, reinterpret_cast<uint32_t>(IDTISR32));
	SetInterrupt(0x33, reinterpret_cast<uint32_t>(IDTISR33));
	SetInterrupt(0x34, reinterpret_cast<uint32_t>(IDTISR34));
	SetInterrupt(0x35, reinterpret_cast<uint32_t>(IDTISR35));
	SetInterrupt(0x36, reinterpret_cast<uint32_t>(IDTISR36));
	SetInterrupt(0x37, reinterpret_cast<uint32_t>(IDTISR37));
	SetInterrupt(0x38, reinterpret_cast<uint32_t>(IDTISR38));
	SetInterrupt(0x39, reinterpret_cast<uint32_t>(IDTISR39));
	SetInterrupt(0x3A, reinterpret_cast<uint32_t>(IDTISR3A));
	SetInterrupt(0x3B, reinterpret_cast<uint32_t>(IDTISR3B));
	SetInterrupt(0x3C, reinterpret_cast<uint32_t>(IDTISR3C));
	SetInterrupt(0x3D, reinterpret_cast<uint32_t>(IDTISR3D));
	SetInterrupt(0x3E, reinterpret_cast<uint32_t>(IDTISR3E));
	SetInterrupt(0x3F, reinterpret_cast<uint32_t>(IDTISR3F));
	SetInterrupt(0x40, reinterpret_cast<uint32_t>(IDTISR40));
	SetInterrupt(0x41, reinterpret_cast<uint32_t>(IDTISR41));
	SetInterrupt(0x42, reinterpret_cast<uint32_t>(IDTISR42));
	SetInterrupt(0x43, reinterpret_cast<uint32_t>(IDTISR43));
	SetInterrupt(0x44, reinterpret_cast<uint32_t>(IDTISR44));
	SetInterrupt(0x45, reinterpret_cast<uint32_t>(IDTISR45));
	SetInterrupt(0x46, reinterpret_cast<uint32_t>(IDTISR46));
	SetInterrupt(0x47, reinterpret_cast<uint32_t>(IDTISR47));
	SetInterrupt(0x48, reinterpret_cast<uint32_t>(IDTISR48));
	SetInterrupt(0x49, reinterpret_cast<uint32_t>(IDTISR49));
	SetInterrupt(0x4A, reinterpret_cast<uint32_t>(IDTISR4A));
	SetInterrupt(0x4B, reinterpret_cast<uint32_t>(IDTISR4B));
	SetInterrupt(0x4C, reinterpret_cast<uint32_t>(IDTISR4C));
	SetInterrupt(0x4D, reinterpret_cast<uint32_t>(IDTISR4D));
	SetInterrupt(0x4E, reinterpret_cast<uint32_t>(IDTISR4E));
	SetInterrupt(0x4F, reinterpret_cast<uint32_t>(IDTISR4F));
	SetInterrupt(0x50, reinterpret_cast<uint32_t>(IDTISR50));
	SetInterrupt(0x51, reinterpret_cast<uint32_t>(IDTISR51));
	SetInterrupt(0x52, reinterpret_cast<uint32_t>(IDTISR52));
	SetInterrupt(0x53, reinterpret_cast<uint32_t>(IDTISR53));
	SetInterrupt(0x54, reinterpret_cast<uint32_t>(IDTISR54));
	SetInterrupt(0x55, reinterpret_cast<uint32_t>(IDTISR55));
	SetInterrupt(0x56, reinterpret_cast<uint32_t>(IDTISR56));
	SetInterrupt(0x57, reinterpret_cast<uint32_t>(IDTISR57));
	SetInterrupt(0x58, reinterpret_cast<uint32_t>(IDTISR58));
	SetInterrupt(0x59, reinterpret_cast<uint32_t>(IDTISR59));
	SetInterrupt(0x5A, reinterpret_cast<uint32_t>(IDTISR5A));
	SetInterrupt(0x5B, reinterpret_cast<uint32_t>(IDTISR5B));
	SetInterrupt(0x5C, reinterpret_cast<uint32_t>(IDTISR5C));
	SetInterrupt(0x5D, reinterpret_cast<uint32_t>(IDTISR5D));
	SetInterrupt(0x5E, reinterpret_cast<uint32_t>(IDTISR5E));
	SetInterrupt(0x5F, reinterpret_cast<uint32_t>(IDTISR5F));
	SetInterrupt(0x60, reinterpret_cast<uint32_t>(IDTISR60));
	SetInterrupt(0x61, reinterpret_cast<uint32_t>(IDTISR61));
	SetInterrupt(0x62, reinterpret_cast<uint32_t>(IDTISR62));
	SetInterrupt(0x63, reinterpret_cast<uint32_t>(IDTISR63));
	SetInterrupt(0x64, reinterpret_cast<uint32_t>(IDTISR64));
	SetInterrupt(0x65, reinterpret_cast<uint32_t>(IDTISR65));
	SetInterrupt(0x66, reinterpret_cast<uint32_t>(IDTISR66));
	SetInterrupt(0x67, reinterpret_cast<uint32_t>(IDTISR67));
	SetInterrupt(0x68, reinterpret_cast<uint32_t>(IDTISR68));
	SetInterrupt(0x69, reinterpret_cast<uint32_t>(IDTISR69));
	SetInterrupt(0x6A, reinterpret_cast<uint32_t>(IDTISR6A));
	SetInterrupt(0x6B, reinterpret_cast<uint32_t>(IDTISR6B));
	SetInterrupt(0x6C, reinterpret_cast<uint32_t>(IDTISR6C));
	SetInterrupt(0x6D, reinterpret_cast<uint32_t>(IDTISR6D));
	SetInterrupt(0x6E, reinterpret_cast<uint32_t>(IDTISR6E));
	SetInterrupt(0x6F, reinterpret_cast<uint32_t>(IDTISR6F));
	SetInterrupt(0x70, reinterpret_cast<uint32_t>(IDTISR70));
	SetInterrupt(0x71, reinterpret_cast<uint32_t>(IDTISR71));
	SetInterrupt(0x72, reinterpret_cast<uint32_t>(IDTISR72));
	SetInterrupt(0x73, reinterpret_cast<uint32_t>(IDTISR73));
	SetInterrupt(0x74, reinterpret_cast<uint32_t>(IDTISR74));
	SetInterrupt(0x75, reinterpret_cast<uint32_t>(IDTISR75));
	SetInterrupt(0x76, reinterpret_cast<uint32_t>(IDTISR76));
	SetInterrupt(0x77, reinterpret_cast<uint32_t>(IDTISR77));
	SetInterrupt(0x78, reinterpret_cast<uint32_t>(IDTISR78));
	SetInterrupt(0x79, reinterpret_cast<uint32_t>(IDTISR79));
	SetInterrupt(0x7A, reinterpret_cast<uint32_t>(IDTISR7A));
	SetInterrupt(0x7B, reinterpret_cast<uint32_t>(IDTISR7B));
	SetInterrupt(0x7C, reinterpret_cast<uint32_t>(IDTISR7C));
	SetInterrupt(0x7D, reinterpret_cast<uint32_t>(IDTISR7D));
	SetInterrupt(0x7E, reinterpret_cast<uint32_t>(IDTISR7E));
	SetInterrupt(0x7F, reinterpret_cast<uint32_t>(IDTISR7F));
	SetInterrupt(0x80, reinterpret_cast<uint32_t>(IDTISR80));
	SetInterrupt(0x81, reinterpret_cast<uint32_t>(IDTISR81));
	SetInterrupt(0x82, reinterpret_cast<uint32_t>(IDTISR82));
	SetInterrupt(0x83, reinterpret_cast<uint32_t>(IDTISR83));
	SetInterrupt(0x84, reinterpret_cast<uint32_t>(IDTISR84));
	SetInterrupt(0x85, reinterpret_cast<uint32_t>(IDTISR85));
	SetInterrupt(0x86, reinterpret_cast<uint32_t>(IDTISR86));
	SetInterrupt(0x87, reinterpret_cast<uint32_t>(IDTISR87));
	SetInterrupt(0x88, reinterpret_cast<uint32_t>(IDTISR88));
	SetInterrupt(0x89, reinterpret_cast<uint32_t>(IDTISR89));
	SetInterrupt(0x8A, reinterpret_cast<uint32_t>(IDTISR8A));
	SetInterrupt(0x8B, reinterpret_cast<uint32_t>(IDTISR8B));
	SetInterrupt(0x8C, reinterpret_cast<uint32_t>(IDTISR8C));
	SetInterrupt(0x8D, reinterpret_cast<uint32_t>(IDTISR8D));
	SetInterrupt(0x8E, reinterpret_cast<uint32_t>(IDTISR8E));
	SetInterrupt(0x8F, reinterpret_cast<uint32_t>(IDTISR8F));
	SetInterrupt(0x90, reinterpret_cast<uint32_t>(IDTISR90));
	SetInterrupt(0x91, reinterpret_cast<uint32_t>(IDTISR91));
	SetInterrupt(0x92, reinterpret_cast<uint32_t>(IDTISR92));
	SetInterrupt(0x93, reinterpret_cast<uint32_t>(IDTISR93));
	SetInterrupt(0x94, reinterpret_cast<uint32_t>(IDTISR94));
	SetInterrupt(0x95, reinterpret_cast<uint32_t>(IDTISR95));
	SetInterrupt(0x96, reinterpret_cast<uint32_t>(IDTISR96));
	SetInterrupt(0x97, reinterpret_cast<uint32_t>(IDTISR97));
	SetInterrupt(0x98, reinterpret_cast<uint32_t>(IDTISR98));
	SetInterrupt(0x99, reinterpret_cast<uint32_t>(IDTISR99));
	SetInterrupt(0x9A, reinterpret_cast<uint32_t>(IDTISR9A));
	SetInterrupt(0x9B, reinterpret_cast<uint32_t>(IDTISR9B));
	SetInterrupt(0x9C, reinterpret_cast<uint32_t>(IDTISR9C));
	SetInterrupt(0x9D, reinterpret_cast<uint32_t>(IDTISR9D));
	SetInterrupt(0x9E, reinterpret_cast<uint32_t>(IDTISR9E));
	SetInterrupt(0x9F, reinterpret_cast<uint32_t>(IDTISR9F));
	SetInterrupt(0xA0, reinterpret_cast<uint32_t>(IDTISRA0));
	SetInterrupt(0xA1, reinterpret_cast<uint32_t>(IDTISRA1));
	SetInterrupt(0xA2, reinterpret_cast<uint32_t>(IDTISRA2));
	SetInterrupt(0xA3, reinterpret_cast<uint32_t>(IDTISRA3));
	SetInterrupt(0xA4, reinterpret_cast<uint32_t>(IDTISRA4));
	SetInterrupt(0xA5, reinterpret_cast<uint32_t>(IDTISRA5));
	SetInterrupt(0xA6, reinterpret_cast<uint32_t>(IDTISRA6));
	SetInterrupt(0xA7, reinterpret_cast<uint32_t>(IDTISRA7));
	SetInterrupt(0xA8, reinterpret_cast<uint32_t>(IDTISRA8));
	SetInterrupt(0xA9, reinterpret_cast<uint32_t>(IDTISRA9));
	SetInterrupt(0xAA, reinterpret_cast<uint32_t>(IDTISRAA));
	SetInterrupt(0xAB, reinterpret_cast<uint32_t>(IDTISRAB));
	SetInterrupt(0xAC, reinterpret_cast<uint32_t>(IDTISRAC));
	SetInterrupt(0xAD, reinterpret_cast<uint32_t>(IDTISRAD));
	SetInterrupt(0xAE, reinterpret_cast<uint32_t>(IDTISRAE));
	SetInterrupt(0xAF, reinterpret_cast<uint32_t>(IDTISRAF));
	SetInterrupt(0xB0, reinterpret_cast<uint32_t>(IDTISRB0));
	SetInterrupt(0xB1, reinterpret_cast<uint32_t>(IDTISRB1));
	SetInterrupt(0xB2, reinterpret_cast<uint32_t>(IDTISRB2));
	SetInterrupt(0xB3, reinterpret_cast<uint32_t>(IDTISRB3));
	SetInterrupt(0xB4, reinterpret_cast<uint32_t>(IDTISRB4));
	SetInterrupt(0xB5, reinterpret_cast<uint32_t>(IDTISRB5));
	SetInterrupt(0xB6, reinterpret_cast<uint32_t>(IDTISRB6));
	SetInterrupt(0xB7, reinterpret_cast<uint32_t>(IDTISRB7));
	SetInterrupt(0xB8, reinterpret_cast<uint32_t>(IDTISRB8));
	SetInterrupt(0xB9, reinterpret_cast<uint32_t>(IDTISRB9));
	SetInterrupt(0xBA, reinterpret_cast<uint32_t>(IDTISRBA));
	SetInterrupt(0xBB, reinterpret_cast<uint32_t>(IDTISRBB));
	SetInterrupt(0xBC, reinterpret_cast<uint32_t>(IDTISRBC));
	SetInterrupt(0xBD, reinterpret_cast<uint32_t>(IDTISRBD));
	SetInterrupt(0xBE, reinterpret_cast<uint32_t>(IDTISRBE));
	SetInterrupt(0xBF, reinterpret_cast<uint32_t>(IDTISRBF));
	SetInterrupt(0xC0, reinterpret_cast<uint32_t>(IDTISRC0));
	SetInterrupt(0xC1, reinterpret_cast<uint32_t>(IDTISRC1));
	SetInterrupt(0xC2, reinterpret_cast<uint32_t>(IDTISRC2));
	SetInterrupt(0xC3, reinterpret_cast<uint32_t>(IDTISRC3));
	SetInterrupt(0xC4, reinterpret_cast<uint32_t>(IDTISRC4));
	SetInterrupt(0xC5, reinterpret_cast<uint32_t>(IDTISRC5));
	SetInterrupt(0xC6, reinterpret_cast<uint32_t>(IDTISRC6));
	SetInterrupt(0xC7, reinterpret_cast<uint32_t>(IDTISRC7));
	SetInterrupt(0xC8, reinterpret_cast<uint32_t>(IDTISRC8));
	SetInterrupt(0xC9, reinterpret_cast<uint32_t>(IDTISRC9));
	SetInterrupt(0xCA, reinterpret_cast<uint32_t>(IDTISRCA));
	SetInterrupt(0xCB, reinterpret_cast<uint32_t>(IDTISRCB));
	SetInterrupt(0xCC, reinterpret_cast<uint32_t>(IDTISRCC));
	SetInterrupt(0xCD, reinterpret_cast<uint32_t>(IDTISRCD));
	SetInterrupt(0xCE, reinterpret_cast<uint32_t>(IDTISRCE));
	SetInterrupt(0xCF, reinterpret_cast<uint32_t>(IDTISRCF));
	SetInterrupt(0xD0, reinterpret_cast<uint32_t>(IDTISRD0));
	SetInterrupt(0xD1, reinterpret_cast<uint32_t>(IDTISRD1));
	SetInterrupt(0xD2, reinterpret_cast<uint32_t>(IDTISRD2));
	SetInterrupt(0xD3, reinterpret_cast<uint32_t>(IDTISRD3));
	SetInterrupt(0xD4, reinterpret_cast<uint32_t>(IDTISRD4));
	SetInterrupt(0xD5, reinterpret_cast<uint32_t>(IDTISRD5));
	SetInterrupt(0xD6, reinterpret_cast<uint32_t>(IDTISRD6));
	SetInterrupt(0xD7, reinterpret_cast<uint32_t>(IDTISRD7));
	SetInterrupt(0xD8, reinterpret_cast<uint32_t>(IDTISRD8));
	SetInterrupt(0xD9, reinterpret_cast<uint32_t>(IDTISRD9));
	SetInterrupt(0xDA, reinterpret_cast<uint32_t>(IDTISRDA));
	SetInterrupt(0xDB, reinterpret_cast<uint32_t>(IDTISRDB));
	SetInterrupt(0xDC, reinterpret_cast<uint32_t>(IDTISRDC));
	SetInterrupt(0xDD, reinterpret_cast<uint32_t>(IDTISRDD));
	SetInterrupt(0xDE, reinterpret_cast<uint32_t>(IDTISRDE));
	SetInterrupt(0xDF, reinterpret_cast<uint32_t>(IDTISRDF));
	SetInterrupt(0xE0, reinterpret_cast<uint32_t>(IDTISRE0));
	SetInterrupt(0xE1, reinterpret_cast<uint32_t>(IDTISRE1));
	SetInterrupt(0xE2, reinterpret_cast<uint32_t>(IDTISRE2));
	SetInterrupt(0xE3, reinterpret_cast<uint32_t>(IDTISRE3));
	SetInterrupt(0xE4, reinterpret_cast<uint32_t>(IDTISRE4));
	SetInterrupt(0xE5, reinterpret_cast<uint32_t>(IDTISRE5));
	SetInterrupt(0xE6, reinterpret_cast<uint32_t>(IDTISRE6));
	SetInterrupt(0xE7, reinterpret_cast<uint32_t>(IDTISRE7));
	SetInterrupt(0xE8, reinterpret_cast<uint32_t>(IDTISRE8));
	SetInterrupt(0xE9, reinterpret_cast<uint32_t>(IDTISRE9));
	SetInterrupt(0xEA, reinterpret_cast<uint32_t>(IDTISREA));
	SetInterrupt(0xEB, reinterpret_cast<uint32_t>(IDTISREB));
	SetInterrupt(0xEC, reinterpret_cast<uint32_t>(IDTISREC));
	SetInterrupt(0xED, reinterpret_cast<uint32_t>(IDTISRED));
	SetInterrupt(0xEE, reinterpret_cast<uint32_t>(IDTISREE));
	SetInterrupt(0xEF, reinterpret_cast<uint32_t>(IDTISREF));
	SetInterrupt(0xF0, reinterpret_cast<uint32_t>(IDTISRF0));
	SetInterrupt(0xF1, reinterpret_cast<uint32_t>(IDTISRF1));
	SetInterrupt(0xF2, reinterpret_cast<uint32_t>(IDTISRF2));
	SetInterrupt(0xF3, reinterpret_cast<uint32_t>(IDTISRF3));
	SetInterrupt(0xF4, reinterpret_cast<uint32_t>(IDTISRF4));
	SetInterrupt(0xF5, reinterpret_cast<uint32_t>(IDTISRF5));
	SetInterrupt(0xF6, reinterpret_cast<uint32_t>(IDTISRF6));
	SetInterrupt(0xF7, reinterpret_cast<uint32_t>(IDTISRF7));
	SetInterrupt(0xF8, reinterpret_cast<uint32_t>(IDTISRF8));
	SetInterrupt(0xF9, reinterpret_cast<uint32_t>(IDTISRF9));
	SetInterrupt(0xFA, reinterpret_cast<uint32_t>(IDTISRFA));
	SetInterrupt(0xFB, reinterpret_cast<uint32_t>(IDTISRFB));
	SetInterrupt(0xFC, reinterpret_cast<uint32_t>(IDTISRFC));
	SetInterrupt(0xFD, reinterpret_cast<uint32_t>(IDTISRFD));
	SetInterrupt(0xFE, reinterpret_cast<uint32_t>(IDTISRFE));
	SetInterrupt(0xFF, reinterpret_cast<uint32_t>(IDTISRFF));