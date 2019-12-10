package com.yx.yxaudioplayerlib.enums;

/**
 * Author by YX, Date on 2019/12/10.
 */
public enum ChannelEnum {

    CHANNEL_LEFT(1),
    CHANNEL_RIGHT(0),
    CHANNEL_STEREO(2);

    private int value;

    ChannelEnum(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

}
