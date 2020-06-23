//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engageandroid;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.rallytac.engage.engine.Engine;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.UUID;

public class CertStoreListActivity extends AppCompatActivity
{
    private static String TAG = CertStoreListActivity.class.getSimpleName();

    private static int PICK_CERTSTORE_FILE_REQUEST_CODE = 42;

    private ArrayList<CertStore> _stores;
    private CertStoreListAdapter _adapter;
    private Intent _resultIntent = new Intent();
    private String _activeCertStoreFileName;

    private class CertStore
    {
        public String _fileName;
        public JSONObject _descriptor;

        private String _cachedDisplayName = null;
        private String _cachedDescription = null;

        public String getDisplayName()
        {
            if(Utils.isEmptyString(_cachedDisplayName))
            {
                _cachedDisplayName = _fileName;

                if (!Utils.isEmptyString(_cachedDisplayName))
                {
                    int pos = _cachedDisplayName.indexOf("}-");
                    _cachedDisplayName = _cachedDisplayName.substring(pos + 2);
                }
                else
                {
                    _cachedDisplayName = "(no name)";
                }
            }

            return _cachedDisplayName;
        }

        public String getDescription()
        {
            if(Utils.isEmptyString(_cachedDescription))
            {
                StringBuilder sb = new StringBuilder();

                sb.append("V");
                sb.append(_descriptor.optInt(Engine.JsonFields.CertStoreDescriptor.version, 0));
                sb.append(", ");

                JSONArray certificates = _descriptor.optJSONArray(Engine.JsonFields.CertStoreDescriptor.certificates);
                sb.append(certificates.length());
                sb.append(" certificates");

                sb.append("\nhash [");
                StringBuilder hashInput = new StringBuilder();
                hashInput.append(_descriptor.optInt(Engine.JsonFields.CertStoreDescriptor.version, 0));
                hashInput.append(certificates.toString());
                sb.append(Utils.md5HashOfString(hashInput.toString()));
                sb.append("]");

                _cachedDescription = sb.toString();
            }

            return _cachedDescription;
        }
    }

    private class CertStoreListAdapter extends ArrayAdapter<CertStore>
    {
        private Context _ctx;
        private int _resId;

        public CertStoreListAdapter(Context ctx, int resId, ArrayList<CertStore> list)
        {
            super(ctx, resId, list);
            _ctx = ctx;
            _resId = resId;
        }

        @NonNull
        @Override
        public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent)
        {
            LayoutInflater inflator = LayoutInflater.from(_ctx);
            convertView = inflator.inflate(_resId, parent, false);

            final CertStore item = getItem(position);

            ((TextView)convertView.findViewById(R.id.tvCertStoreName)).setText(item.getDisplayName());
            ((TextView)convertView.findViewById(R.id.tvDescription)).setText(item.getDescription());

            convertView.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    viewCertStore(item);
                }
            });

            convertView.findViewById(R.id.ivDeleteCertStore).setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    confirmDeleteCertStore(item);
                }
            });

            if (_activeCertStoreFileName.compareTo(item._fileName) == 0)
            {
                ((ImageView) convertView.findViewById(R.id.ivActiveCertStoreIndicator))
                        .setImageDrawable(ContextCompat.getDrawable(CertStoreListActivity.this,
                                R.drawable.ic_radio_button_selected));
            }
            else
            {
                ((ImageView) convertView.findViewById(R.id.ivActiveCertStoreIndicator))
                        .setImageDrawable(ContextCompat.getDrawable(CertStoreListActivity.this,
                                R.drawable.ic_radio_button_unselected));
            }

            convertView.findViewById(R.id.ivActiveCertStoreIndicator).setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    activateCertStore(item);
                }
            });

            return convertView;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_certstore_list);

        FloatingActionButton fabAdd = findViewById(R.id.fabAdd);
        fabAdd.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                selectFileForImport();
            }
        });

        _activeCertStoreFileName = Globals.getSharedPreferences().getString(PreferenceKeys.USER_CERT_STORE_FILE_NAME, "");

        loadStores(Globals.getEngageApplication().getCertStoreCacheDir());

        _adapter = new CertStoreListAdapter(this, R.layout.certstore_list_entry, _stores);
        ListView lv = findViewById(R.id.lvCertStores);
        lv.setAdapter(_adapter);

        setupActionBar();
    }

    private void setupActionBar()
    {
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null)
        {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        int id = item.getItemId();
        if (id == android.R.id.home)
        {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        Log.d(TAG, "onActivityResult");

        if(resultCode == RESULT_OK)
        {
            if(requestCode == PICK_CERTSTORE_FILE_REQUEST_CODE)
            {
                importCertStoreFromUri(intent.getData());
            }
        }
    }

    private void loadStores(String sourceDirectory)
    {
        _stores = new ArrayList<>();

        try
        {
            File dir = new File(sourceDirectory);
            File[] allContents = dir.listFiles();
            if (allContents != null)
            {
                for (File file : allContents)
                {
                    CertStore cs = loadStoreFrom(file.getAbsolutePath());
                    if(cs != null)
                    {
                        _stores.add(cs);
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void selectFileForImport()
    {
        try
        {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("*/*");
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            startActivityForResult(Intent.createChooser(intent, "Select a File"), PICK_CERTSTORE_FILE_REQUEST_CODE);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void importCertStoreFromUri(Uri uri)
    {
        if(uri == null)
        {
            return;
        }

        try
        {
            String displayName;
            String importedFileName;

            displayName = uri.getPath();
            displayName = displayName.substring(displayName.lastIndexOf('/') + 1);

            importedFileName = "{" + UUID.randomUUID().toString() + "}-" + displayName;

            File fo = File.createTempFile("import-", "-import", getCacheDir());
            String fn = fo.getAbsolutePath();
            fo.deleteOnExit();
            BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(fo));

            BufferedInputStream bis = new BufferedInputStream(getContentResolver().openInputStream(uri));
            byte[] data = new byte[8192];
            int numRead;

            while( (numRead = bis.read(data)) > 0 )
            {
                bos.write(data, 0, numRead);
            }

            bis.close();
            bos.close();

            importCertStoreFromFile(fn, importedFileName);

            fo.delete();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void importCertStoreFromFile(String fn, String importedFileName)
    {
        if(Utils.isEmptyString(fn))
        {
            return;
        }

        try
        {
            String descriptorText = Globals.getEngageApplication().getEngine().engageQueryCertStoreContents(fn, "");
            if(Utils.isEmptyString(descriptorText))
            {
                Utils.showLongPopupMsg(this, "Invalid certificate store - cannot obtain the descriptor");
                throw new Exception("");
            }

            JSONObject descriptor = null;

            try
            {
                descriptor = new JSONObject(descriptorText);
                if(descriptor == null)
                {
                    throw new Exception("");
                }
            }
            catch (Exception e)
            {
                Utils.showLongPopupMsg(this, "Invalid certificate store - cannot decode the descriptor");
                throw e;
            }

            int version = descriptor.optInt(Engine.JsonFields.CertStoreDescriptor.version, 0);
            if(version <= 0)
            {
                Utils.showLongPopupMsg(this, "Invalid certificate store - unknown version " + version);
                throw new Exception("");
            }

            JSONArray certificates = null;
            try
            {
                certificates = descriptor.getJSONArray(Engine.JsonFields.CertStoreDescriptor.certificates);
                if(certificates == null)
                {
                    throw new Exception("");
                }

                if(certificates.length() <= 0)
                {
                    throw new Exception("");
                }
            }
            catch (Exception e)
            {
                Utils.showLongPopupMsg(this, "Invalid certificate store - no certificates found");
                throw e;
            }

            try
            {
                // Copy the file into our cache
                String path = Globals.getEngageApplication().getCertStoreCacheDir() + "/" + importedFileName;
                File fo = new File(path);
                fo.createNewFile();
                BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(fo));
                File fi = new File(fn);
                BufferedInputStream bis = new BufferedInputStream(new FileInputStream(fi));
                byte[] data = new byte[2048];
                int numRead;
                while ((numRead = bis.read(data)) > 0)
                {
                    bos.write(data, 0, numRead);
                }
                bis.close();
                bos.close();

                // Add the certstore to our list of stores
                CertStore cs = loadStoreFrom(fo.getAbsolutePath());
                if(cs != null)
                {
                    _stores.add(cs);
                    _adapter.notifyDataSetChanged();
                }
            }
            catch (Exception e)
            {
                Utils.showLongPopupMsg(this, "I/O error encountered while importing");
                throw e;
            }
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    private CertStore loadStoreFrom(String fn)
    {
        CertStore rc = new CertStore();

        try
        {
            rc._fileName = fn;
            rc._descriptor = Globals.getEngageApplication().getCertificateStoreDescriptorForFile(fn);
            if(rc._descriptor == null)
            {
                throw new Exception("Cannot get certificate store descriptor");
            }
        }
        catch (Exception e)
        {
            rc = null;
            e.printStackTrace();
        }

        return rc;
    }

    private void activateCertStore(final CertStore cs)
    {
        _activeCertStoreFileName = (cs == null ? "" : cs._fileName);

        SharedPreferences.Editor ed = Globals.getSharedPreferencesEditor();
        ed.putString(PreferenceKeys.USER_CERT_STORE_FILE_NAME, _activeCertStoreFileName);
        ed.apply();

        _resultIntent.putExtra(Constants.CERTSTORE_CHANGED_TO_FN, _activeCertStoreFileName);
        setResult(RESULT_OK, _resultIntent);
        finish();
    }

    private void viewCertStore(final CertStore cs)
    {
        // TODO
    }

    private void confirmDeleteCertStore(final CertStore cs)
    {
        final boolean isActive = (cs._fileName.compareTo(_activeCertStoreFileName) == 0);
        String s;

        if(isActive)
        {
            s = "This certificate store is currently in use.  If you delete it, the application will use the default certificate store until you select another.\n\nProceed?";
        }
        else
        {
            s = "Are you sure you want to delete this certificate store?";
        }

        final TextView message = new TextView(this);
        final SpannableString ss = new SpannableString(s);

        message.setText(ss);
        message.setMovementMethod(LinkMovementMethod.getInstance());
        message.setPadding(32, 32, 32, 32);

        AlertDialog dlg = new AlertDialog.Builder(this)
                .setTitle("Delete Certificate Store")
                .setCancelable(false)
                .setPositiveButton("Yes", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i)
                    {
                        for(CertStore c : _stores)
                        {
                            if(c._fileName.compareTo(cs._fileName) == 0)
                            {
                                try
                                {
                                    File f = new File(cs._fileName);
                                    f.delete();
                                    _stores.remove(c);
                                    _adapter.notifyDataSetChanged();

                                    if(isActive)
                                    {
                                        activateCertStore(null);
                                    }
                                }
                                catch (Exception e)
                                {
                                    e.printStackTrace();
                                }

                                break;
                            }
                        }
                    }
                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i)
                    {
                    }
                }).setView(message).create();

        dlg.show();
    }
}
