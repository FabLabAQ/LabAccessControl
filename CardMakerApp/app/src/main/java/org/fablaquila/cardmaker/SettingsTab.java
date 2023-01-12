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

import static java.lang.Integer.parseInt;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.preference.EditTextPreference;
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
import android.widget.CheckBox;
import android.widget.EditText;
import android.util.Log;

import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.KeyPair;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableEntryException;
import java.security.cert.CertificateException;
import java.util.Random;

import javax.crypto.Mac;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

public class SettingsTab extends Fragment {

    private View view;
    private Button desKeyBtn, genSignKeyBtn, scriptIdBtn;
    private EditText desKeyText, scriptIdText, adminIdText;
    private Context context;
    private MainActivity activity;
    private AlertDialog keyAlertDialog;
    private MainTab mainTab;
    //private Switch restoreSwitch;
    private SharedPreferences sharedPref;
    private ClipboardManager clipboard;
    private KeyStore keyStore;
    public static CheckBox dryrun;

    private static final String aliasPrefix = "org.fablaquila.cardmaker.";
    public static final String desKeyAlias = aliasPrefix + "DesKey";
    public static final String ANDROID_KEYSTORE = "AndroidKeyStore";
    public static final String scriptIdAlias = aliasPrefix + "ScriptId";
    public static final String hmacKeyAlias = aliasPrefix + "HmacKey";
    public static final String adminIdAlias = aliasPrefix + "AdminId";
    public static SecretKey desKey;
    public static SecretKey signKey;
    public static String scriptId;
    public static int adminId;

    private boolean desKeySet = false, signKeySet = false, adminIdSet = false, scriptIdSet = false, settingsReady=false;

    public SettingsTab() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activity = (MainActivity) getActivity();
        context = getContext();
        sharedPref = activity.getPreferences(Context.MODE_PRIVATE);
        //scriptIdRetrieve();
        clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
        try {
            keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
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
        desKeyBtn = view.findViewById(R.id.desKeyButton);
        desKeyText = view.findViewById(R.id.desKeyTextInput);
        //restoreSwitch = view.findViewById(R.id.restoreSwitch);
        genSignKeyBtn = view.findViewById(R.id.genSignKeyButton);
        scriptIdText = view.findViewById(R.id.scriptIdTextInput);
        scriptIdBtn = view.findViewById(R.id.scriptIdButton);
        adminIdText = view.findViewById(R.id.adminIdEditText);

        dryrun = view.findViewById(R.id.dryRunCheckBox);
        mainTab = activity.mainTab;

        desKeyBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                hideKeyboard();

                // test key: KhGJGmHMHDS0FubbcqiVnAqxoRv1SlqD0Mg/EtXrdb0=
                if (desKeyText.getText().toString().length() != 44) {
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
                                    byte[] key = Base64.decode(desKeyText.getText().toString(), Base64.DEFAULT);
                                    if (desKeySet(key)) {
                                        key = null;
                                        desKeySet = true;
                                        note(getString(R.string.des_key_updated));
                                        desKeyText.setHint(getString(R.string.des_key_set));
                                        desKeyBtn.setText(R.string.change);
                                    }
                                    else {
                                        note(getString(R.string.des_key_error));
                                    }
                                    desKeyText.setText("");
                                }
                            });
                    keyAlertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, getString(R.string.no),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.cancel();
                                    desKeyText.setText("");
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

        genSignKeyBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                hideKeyboard();
                if(signKeySet){
                    changeSignKey();
                }
                else {
                    newSignKey();
                }
            }
        });

        adminIdText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus){
                try {
                    if(!hasFocus) {
                        int value = parseInt(adminIdText.getText().toString());
                        if (value > 0) {
                            adminIdSet = true;
                            sharedPref.edit().putInt(adminIdAlias, value).apply();
                        } else {
                            adminIdText.setHint(getString(R.string.not_set));
                        }
                    }
                }
                catch (Exception e){
                    Log.d("SETTINGS", e.toString());
                    e.printStackTrace();
                }
            }
        });

        // AKfycbxY3IqmoJ4GJ-r2spqr0C0XfgLctTzMxr1SqnbzB1IPC1621MGA
        // AKfycbxfTM0LCk87DG-fjmxkYB0Bsh6VXACkfzhrhyNu8aP4T0MTgTfy
        scriptIdBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                hideKeyboard();
                if (scriptIdText.getText().toString().length() != 56) {
                    note(getString(R.string.wrong_script_id));
                }
                else {
                    scriptId = scriptIdText.getText().toString();
                    sharedPref.edit().putString(scriptIdAlias, scriptId).apply();
                    scriptIdBtn.setText(getString(R.string.change));
                    note(getString(R.string.script_id_set));
                }
            }
        });

        if (desKeyRetrieve()) {
            desKeyText.setHint(getString(R.string.des_key_set));
            desKeyBtn.setText(R.string.change);
            desKeySet = true;

        }
        else {
            desKeyText.setHint(getString(R.string.des_key_not_set));
        }

        if(signKeyRetrieve()){
            signKeySet=true;
            genSignKeyBtn.setText(R.string.change);

        }

        if (scriptIdRetrieve()) {
            scriptIdText.setHint(scriptId);
            scriptIdBtn.setText(R.string.change);
            scriptIdSet = true;
        }
        else {
            scriptIdText.setHint(getString(R.string.script_id_not_set));
        }

        if(sharedPref.contains(adminIdAlias)){
            adminId = sharedPref.getInt(adminIdAlias, -1);
            adminIdSet = true;
            String idString = "" + adminId;
            adminIdText.setText(idString);
        }
        else{
            adminIdText.setHint(getString(R.string.not_set));
            adminIdSet = false;
        }

        if (!(desKeySet && scriptIdSet && signKeySet && adminIdSet)) {
            settingsReady = false;
            setUserVisibleHint(true);
            activity.tabLayout.getTabAt(1).select();
            mainTab.setMode(MainTab.Mode.MODE_INACTIVE);
        }
        else{
            settingsReady = true;
            setUserVisibleHint(false);
        }

        return view;
    }

    private boolean scriptIdRetrieve() {
        if (sharedPref.contains(scriptIdAlias)) {
            scriptId = sharedPref.getString(scriptIdAlias, "");
            return true;
        }
        return false;
    }


    private void changeSignKey() {
        keyAlertDialog = new AlertDialog.Builder(context).create();
        keyAlertDialog.setTitle(getString(R.string.confirm_key_change));
        keyAlertDialog.setMessage(getString(R.string.alert_dialog_msg));
        keyAlertDialog.setButton(AlertDialog.BUTTON_POSITIVE, getString(R.string.yes),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        if(newSignKey()) {
                            signKeySet = true;
                            note(getString(R.string.sign_key_updated));

                        }
                        else{
                            note(getString(R.string.key_gen_error));
                        }

                    }
                });
        keyAlertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, getString(R.string.no),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                        desKeyText.setText("");
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

    private boolean newSignKey() {
        try {
            byte[] key = new byte[32];
            new SecureRandom().nextBytes(key);
            String keyBase64 = Base64.encodeToString(key, Base64.DEFAULT);
            Log.i("SETTINGS", "new sign key: " + keyBase64);
            signKey = new SecretKeySpec(key, KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            KeyStore.SecretKeyEntry secretKeyEntry = new KeyStore.SecretKeyEntry(signKey);
            KeyProtection keyProtection = new KeyProtection.Builder(KeyProperties.PURPOSE_SIGN)
                    //.setDigests(KeyProperties.DIGEST_SHA256)
                    .setInvalidatedByBiometricEnrollment(true)
                    .build();
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            keyStore.setEntry(hmacKeyAlias, secretKeyEntry, keyProtection);
            ClipData clip = ClipData.newPlainText("SignKey", keyBase64);
            clipboard.setPrimaryClip(clip);
            note(getString(R.string.copied_to_clip));
            genSignKeyBtn.setText(R.string.change);
            return true;
        }
        catch (KeyStoreException | CertificateException | IOException | NoSuchAlgorithmException e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean desKeySet(byte[] key) {
        try {
            desKey = new SecretKeySpec(key, KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            KeyStore.SecretKeyEntry secretKeyEntry = new KeyStore.SecretKeyEntry(desKey);
            KeyProtection keyProtection = new KeyProtection.Builder(KeyProperties.PURPOSE_SIGN)
                    //.setDigests(KeyProperties.DIGEST_SHA256)
                    .setInvalidatedByBiometricEnrollment(true)
                    .build();
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            keyStore.setEntry(desKeyAlias, secretKeyEntry, keyProtection);

            return true;
        }
        catch (KeyStoreException | CertificateException | IOException | NoSuchAlgorithmException e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean desKeyRetrieve() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            desKey =  (SecretKey) keyStore.getKey(desKeyAlias, null);
            Mac mac = Mac.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            mac.init(desKey);
            byte[] tempData = new byte[8];
            new Random().nextBytes(tempData);
            mac.doFinal(tempData);
            return true;
        }
        catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean signKeyRetrieve() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            signKey =  (SecretKey) keyStore.getKey(hmacKeyAlias, null);
            Mac mac = Mac.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            mac.init(signKey);
            byte[] tempData = new byte[8];
            new Random().nextBytes(tempData);
            mac.doFinal(tempData);
            return true;
        }
        catch (Exception e) {
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
                if(settingsReady) mainTab.setMode(MainTab.Mode.MODE_ACTIVE);
            }
        }
    }

/*    public boolean restore() {
        return restoreSwitch.isChecked();
    }*/

}
