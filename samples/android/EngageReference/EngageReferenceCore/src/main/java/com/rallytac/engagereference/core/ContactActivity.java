//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.MenuItem;
import android.widget.TextView;

public class ContactActivity extends AppCompatActivity
{
    private static String TAG = ContactActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_contact);

        ActionBar ab = getSupportActionBar();
        if(ab != null)
        {
            ab.setDisplayHomeAsUpEnabled(true);
        }

        convertViewToLink(R.id.tvWebsite, null);
        convertViewToLink(R.id.tvSupport, "mailto:");
        convertViewToLink(R.id.tvSales, "mailto:");
    }

    private void convertViewToLink(int id, String hrefAction)
    {
        TextView tv;
        String s;
        Spanned spanned;
        String url;

        tv = findViewById(id);
        s = tv.getText().toString();

        if(!Utils.isEmptyString(hrefAction))
        {
            url = hrefAction + s;
        }
        else
        {
            url = s;
        }

        spanned = Html.fromHtml("<a href='" + url + "'>" + s + "</a>");
        tv.setText(spanned);
        tv.setMovementMethod(LinkMovementMethod.getInstance());
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
            case android.R.id.home:
                finish();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

}
