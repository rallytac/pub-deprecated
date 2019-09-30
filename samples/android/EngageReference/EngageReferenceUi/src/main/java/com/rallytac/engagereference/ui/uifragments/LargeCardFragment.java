//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.ui.uifragments;
import com.rallytac.engagereference.ui.LcarsActivity;
import com.rallytac.engagereference.ui.R;

public class LargeCardFragment extends CardFragment
{
    private static String TAG = LargeCardFragment.class.getSimpleName();

    @Override
    protected int getLayoutId()
    {
        return R.layout.fragment_large_card;
    }

    @Override
    protected int getCardResId(boolean secure)
    {
        return (secure ? R.drawable.single_channel_background_secure_idle : R.drawable.single_channel_background_clear_idle);
    }

    @Override
    protected int getCardTxResId(boolean secure)
    {
        return (secure ? R.drawable.single_channel_background_secure_tx : R.drawable.single_channel_background_clear_tx);
    }

    @Override
    protected void onCardDoubleTap()
    {
        goToMultiView();
    }

    @Override
    protected void onCardSwiped(GestureDirection direction)
    {
        if(direction == GestureDirection.gdLeft || direction == GestureDirection.gdRight)
        {
            goToMultiView();
        }
    }

    private void goToMultiView()
    {
        ((LcarsActivity) getActivity()).showMultiView();
    }
}
