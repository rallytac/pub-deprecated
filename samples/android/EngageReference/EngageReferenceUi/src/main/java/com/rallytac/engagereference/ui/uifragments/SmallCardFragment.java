//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.ui.uifragments;
import com.rallytac.engagereference.ui.LcarsActivity;
import com.rallytac.engagereference.ui.R;

public class SmallCardFragment extends CardFragment
{
    private static String TAG = SmallCardFragment.class.getSimpleName();

    @Override
    protected int getLayoutId()
    {
        return R.layout.fragment_small_card;
    }

    @Override
    protected int getCardResId(boolean secure)
    {
        return (secure ? R.drawable.multi_channel_background_secure_idle : R.drawable.multi_channel_background_clear_idle);
    }

    @Override
    protected int getCardTxResId(boolean secure)
    {
        return (secure ? R.drawable.multi_channel_background_secure_tx : R.drawable.multi_channel_background_clear_tx);
    }

    @Override
    protected void onCardDoubleTap()
    {
        goToSingleViewForThisCard();
    }

    @Override
    protected void onCardSwiped(GestureDirection direction)
    {
        if(direction == GestureDirection.gdLeft || direction == GestureDirection.gdRight)
        {
            goToSingleViewForThisCard();
        }
    }

    private void goToSingleViewForThisCard()
    {
        if(_gd != null)
        {
            ((LcarsActivity) getActivity()).showSingleView(_gd.id);
        }
    }
}
