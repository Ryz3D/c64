unsigned char ram[4096] = {
    // $c000 - $d000
    0xa9, 0x00, // lda #$00
    0xaa,       // tax
    0xa8,       // tay
    0xe8,       // inx
    0xe0, 0x1a, // cpx #$1a
    0xd0, 0xfb, // bne
    0xc8,       // iny
};
