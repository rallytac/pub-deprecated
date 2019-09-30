//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

public interface IPushToTalkRequestHandler
{
    void requestPttOn(int priority, int flags);
    void requestPttOff();
}
