/*
 * (c) 2019 Luca Anastasio
 * anastasio.lu@gmail.com
 * www.fablaquila.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package org.fablaquila.cardmaker;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.security.keystore.KeyProperties;
import android.security.keystore.KeyProtection;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Base64;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;

import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableEntryException;
import java.security.cert.CertificateException;
import java.util.Random;

import javax.crypto.Mac;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

public class SettingsTab extends Fragment {

    private View view;
    private Button setMasterKeyBtn;
    private EditText masterKeyText;
    private Context context;
    private MainActivity activity;
    private AlertDialog keyAlertDialog;
    private MainTab mainTab;
    //private Switch restoreSwitch;


    private static final String aliasPrefix = "cardmaker";
    public static final String masterKeyAlias = aliasPrefix + "MasterKey";
    public static final String ANDROID_KEYSTORE = "AndroidKeyStore";
    public static SecretKey masterKey;

    private boolean masterKeySet = false;

    public SettingsTab() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activity = (MainActivity) getActivity();
        context = getContext();
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
        } catch (KeyStoreException | CertificateException | IOException | NoSuchAlgorithmException e) {
            e.printStackTrace();
        }

    }

    private void hideKeyboard() {
        InputMethodManager imm = (InputMethodManager) context.getSystemService(Activity.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }

    private void note(String s) {
        Snackbar.make(view.getRootView(), s, Snackbar.LENGTH_LONG).show();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.settingstab, container, false);
        setMasterKeyBtn = view.findViewById(R.id.setMasterKeyButton);
        masterKeyText = view.findViewById(R.id.masterKeyTextInput);
        //restoreSwitch = view.findViewById(R.id.restoreSwitch);
        mainTab = activity.mainTab;

        setMasterKeyBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                hideKeyboard();
                if (masterKeyText.getText().toString().length() != 44) {
                    note(getString(R.string.wrong_key_size));
                }
                else {
                    keyAlertDialog = new AlertDialog.Builder(context).create();
                    keyAlertDialog.setTitle(getString(R.string.confirm_key_change));
                    keyAlertDialog.setMessage(getString(R.string.alert_dialog_msg));
                    keyAlertDialog.setButton(AlertDialog.BUTTON_POSITIVE, getString(R.string.yes),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.dismiss();
                                    byte[] key = Base64.decode(masterKeyText.getText().toString(), Base64.DEFAULT);
                                    if (setMasterKey(key)) {
                                        key = null;
                                        masterKeySet = true;
                                        note(getString(R.string.master_key_updated));
                                        masterKeyText.setHint(getString(R.string.master_key_set));
                                        setMasterKeyBtn.setText(R.string.change);
                                    }
                                    else {
                                        note(getString(R.string.master_key_error));
                                    }
                                    masterKeyText.setText("");
                                }
                            });
                    keyAlertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, getString(R.string.no),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.cancel();
                                    masterKeyText.setText("");
                                    note(getString(R.string.canceled));
                                }
                            });
                    keyAlertDialog.setOnShowListener( new DialogInterface.OnShowListener() {
                        @Override
                        public void onShow(DialogInterface arg0) {
                            keyAlertDialog.getButton(AlertDialog.BUTTON_POSITIVE).
                                    setTextColor(getResources().getColor(R.color.design_default_color_primary));
                            keyAlertDialog.getButton(AlertDialog.BUTTON_NEGATIVE).
                                    setTextColor(getResources().getColor(R.color.design_default_color_primary));
                        }
                    });
                    keyAlertDialog.show();
                }
            }
        });

        if (retrieveMasterKey()) {
            masterKeyText.setHint(getString(R.string.master_key_set));
            setMasterKeyBtn.setText(R.string.change);
            masterKeySet = true;
            setUserVisibleHint(false);
        }
        else {
            activity.tabLayout.getTabAt(1).select();
            masterKeyText.setHint(getString(R.string.master_key_not_set));
            mainTab.setMode(MainTab.Mode.MODE_INACTIVE);
        }
        return view;
    }

    private boolean setMasterKey(byte[] key) {
        try {
            masterKey = new SecretKeySpec(key, KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            KeyStore.SecretKeyEntry secretKeyEntry = new KeyStore.SecretKeyEntry(masterKey);
            KeyProtection keyProtection = new KeyProtection.Builder(KeyProperties.PURPOSE_SIGN)
                    //.setDigests(KeyProperties.DIGEST_SHA256)
                    .setInvalidatedByBiometricEnrollment(false)
                    .build();
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            keyStore.setEntry(masterKeyAlias, secretKeyEntry, keyProtection);

            return true;
        }
        catch (KeyStoreException | CertificateException | IOException | NoSuchAlgorithmException e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean retrieveMasterKey() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            masterKey =  (SecretKey) keyStore.getKey(masterKeyAlias, null);
            Mac mac = Mac.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            mac.init(masterKey);
            byte[] tempData = new byte[8];
            new Random().nextBytes(tempData);
            mac.doFinal(tempData);
            return true;
        }
        catch (KeyStoreException | NoSuchAlgorithmException | InvalidKeyException | UnrecoverableEntryException | CertificateException | IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if(mainTab != null) {
            if (isVisibleToUser) mainTab.setMode(MainTab.Mode.MODE_INACTIVE);
            else {
                if (masterKeySet) mainTab.setMode(MainTab.Mode.MODE_ACTIVE);
            }
        }
    }

/*    public boolean restore() {
        return restoreSwitch.isChecked();
    }*/

}
