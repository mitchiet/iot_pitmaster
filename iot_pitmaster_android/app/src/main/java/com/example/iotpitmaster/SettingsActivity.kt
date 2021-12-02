package com.example.iotpitmaster

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.*
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.view.View
import android.widget.*
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.SimpleItemAnimator
import kotlinx.android.synthetic.main.activity_settings.*


private const val LOCATION_PERMISSION_REQUEST_CODE = 2

class SettingsActivity : AppCompatActivity() {

    private var fahrenheit: Boolean = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_settings)

        val prefs = getSharedPreferences(
            "Note to self",
            Context.MODE_PRIVATE
        )

        //checks the saved preference and changes selected button to the saved preference
        fahrenheit = prefs.getBoolean("units", true)
        radioButtonF.isChecked = fahrenheit
        radioButtonC.isChecked = !fahrenheit

        //changes variable fahrenheit to be saved based on the selected radio button
        temp_units.setOnCheckedChangeListener(RadioGroup.OnCheckedChangeListener() {
            temp_units, _ ->
                fahrenheit = when(temp_units.checkedRadioButtonId) {
                    R.id.radioButtonF -> {
                        true
                    } else -> {
                        false
                    }
                }
        })

        //update Bluetooth Field
        if(MainActivity.m_isConnected) {
            ("Connected with " + MainActivity.m_address).also { bluetoothStatus.text = it }
        } else {
            "Not Connected".also { bluetoothStatus.text = it }
        }
    }

    override fun onPause() {
        super.onPause()

        //save temperature unit settings
        val prefs = getSharedPreferences(
            "Note to self",
            Context.MODE_PRIVATE)
        val editor = prefs.edit()
        editor.putBoolean("units", fahrenheit)
        editor.apply()
    }



    //Lets the user know when temperature units have been changed
    fun temperatureUnits(v: View) {
        val radio: RadioButton = findViewById(temp_units.checkedRadioButtonId)
        Toast.makeText(applicationContext, "Temperature Units are ${radio.text}",
            Toast.LENGTH_SHORT).show()
    }
}