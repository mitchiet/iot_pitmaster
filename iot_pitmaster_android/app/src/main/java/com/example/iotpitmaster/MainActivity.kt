package com.example.iotpitmaster

import android.annotation.SuppressLint
import android.app.Activity
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.bluetooth.*
import android.content.*
import android.content.pm.PackageManager
import android.os.AsyncTask
import android.os.Build
import android.os.Bundle
import android.os.ParcelUuid
import android.provider.Settings
import android.text.format.DateUtils
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.*
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.content.ContextCompat
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView
import kotlinx.android.synthetic.main.content_auto.*
import kotlinx.android.synthetic.main.content_manual.*
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.IOException
import java.nio.ByteBuffer
import java.time.Duration
import java.util.*
import java.time.LocalTime

private const val ENABLE_BLUETOOTH_REQUEST_CODE = 1

open class MainActivity : AppCompatActivity(), NavigationView.OnNavigationItemSelectedListener {

    private var fahrenheit: Boolean = true
    var device: BluetoothDevice? = null
    private val CHANNEL_ID = "channel_id"
    private var notificationId = 101
    private var notificationText = ""
    private var userCookTemp = 315.0F
    private var userChamberTemp = 315.0F
    private var n1_time : LocalTime? = null
    private var n2_time : LocalTime? = null
    private var n3_time : LocalTime? = null

    companion object {
        var my_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
        var m_bluetoothSocket: BluetoothSocket? = null
        var m_bluetoothAdapter: BluetoothAdapter? = null
        var m_isConnected: Boolean = false
        var m_address: String? = null
        var m_fragment: String? = "a"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        //starts the application on the autonomous mode
        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                .replace(R.id.fragmentHolder, AutoFragment()).commit()
        }

        // check for Bluetooth functionality
        m_bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        if (m_bluetoothAdapter == null) {
            Toast.makeText(
                applicationContext,
                "This device doesn't support Bluetooth. This app will not work",
                Toast.LENGTH_LONG
            ).show()
        }
        if (!m_bluetoothAdapter!!.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            startActivityForResult(enableBtIntent, ENABLE_BLUETOOTH_REQUEST_CODE)
        }

        val toolbar = findViewById<androidx.appcompat.widget.Toolbar>(R.id.toolbar)
        setSupportActionBar(toolbar)

        val nav_view = findViewById<NavigationView>(R.id.nav_view)
        val drawer_layout = findViewById<DrawerLayout>(R.id.drawer_layout)

        val toggle = ActionBarDrawerToggle(
            this,
            drawer_layout,
            toolbar,
            R.string.navigation_drawer_open,
            R.string.navigation_drawer_close
        )
        drawer_layout.addDrawerListener(toggle)
        toggle.syncState()

        nav_view.setNavigationItemSelectedListener(this)

        createNotificationChannel()

        //get the address of the IoT Pitmaster ESP
        m_address = getAddress()

        //connect to ESP-32
        Connect(this).execute()
    }

    private fun createNotificationChannel() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "IoT Pitmaster"
            val descriptionText = "Test"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel(CHANNEL_ID, name, importance).apply {
                description = notificationText
            }
            val notificationManager : NotificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }

    private fun sendNotification() {
        val intent = Intent(this, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent: PendingIntent = PendingIntent.getActivity(this, 0, intent, 0)

        val builder = NotificationCompat.Builder(this, CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentTitle("IoT Pitmaster")
            .setContentText(notificationText)
            .setContentIntent(pendingIntent)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)

        with(NotificationManagerCompat.from(this)) {
            notify(notificationId, builder.build())
        }
    }

    private fun notificationTimer(id:Int) {
        val currentTime = LocalTime.now()
        when(id) {
            //chamber temperature running hot
            1 -> {
                val between : Duration = Duration.ZERO
                if(n1_time != null) {
                    Duration.between(n1_time, currentTime)
                }
                if(n1_time == null || between.seconds > 300) {
                    n1_time = LocalTime.now()
                    sendNotification()
                }
            }
            //food is ready
            2 -> {
                val between : Duration = Duration.ZERO
                if(n2_time != null) {
                    Duration.between(n2_time, currentTime)
                }
                if(n2_time == null || between.seconds > 300) {
                    n2_time = LocalTime.now()
                    sendNotification()
                }
            }
            //add more fuel
            3 -> {
                val between : Duration = Duration.ZERO
                if(n3_time != null) {
                    Duration.between(n3_time, currentTime)
                }
                if(n3_time == null || between.seconds > 300) {
                    n3_time = LocalTime.now()
                    sendNotification()
                }
            }
        }
    }

    //asynchronously connect to ESP-32
    private class Connect(c: Context) : AsyncTask<Void, Void, String>() {
        private var connectSuccess: Boolean = true
        private val context: Context

        init {
            this.context = c
        }

        override fun doInBackground(vararg p0: Void?): String? {
            try {
                if (m_bluetoothSocket == null || !m_isConnected) {
                    m_bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
                    val device: BluetoothDevice = m_bluetoothAdapter!!.getRemoteDevice(m_address)
                    m_bluetoothSocket = device.createInsecureRfcommSocketToServiceRecord(my_UUID)
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery() //stop looking for devices to connect to
                    m_bluetoothSocket!!.connect()
                }
            } catch (e: IOException) {
                connectSuccess = false
                e.printStackTrace()
            }
            return null
        }

        override fun onPostExecute(result: String?) {
            super.onPostExecute(result)
            if(!connectSuccess) {
                Log.i("Bluetooth", "couldn't connect")
            } else {
                m_isConnected = true
            }
        }
    }

    private fun getAddress() : String? {
        var address: String? = null
        var uuids: Array<ParcelUuid>? = null
        val pairedDevices: Set<BluetoothDevice> = m_bluetoothAdapter!!.bondedDevices
        for (item in pairedDevices) {
            if (item.name == "IoT Pitmaster") {
                address = item.address
            }
        }
        return address
    }

    override fun onDestroy() {
        super.onDestroy()

        m_bluetoothSocket!!.close()
        m_bluetoothSocket = null
        m_isConnected = false
    }

    override fun onResume() {
        super.onResume()

        //check if user changed temperature unit settings
        val prefs = getSharedPreferences(
            "Note to self",
            Context.MODE_PRIVATE
        )
        fahrenheit = prefs.getBoolean("units", true)

        GlobalScope.launch {
            readBluetoothData()
        }
    }

    override fun onPause() {
        super.onPause()

        GlobalScope.cancel()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        when (requestCode) {
            ENABLE_BLUETOOTH_REQUEST_CODE -> {
                if (resultCode == Activity.RESULT_OK) {
                    if (m_bluetoothAdapter!!.isEnabled) {
                        Toast.makeText(applicationContext,"Bluetooth has been enabled", Toast.LENGTH_SHORT).show()
                    } else {
                        Toast.makeText(applicationContext, "Bluetooth has been disabled", Toast.LENGTH_SHORT).show()
                    }
                } else if (resultCode == Activity.RESULT_CANCELED) {
                    Toast.makeText(applicationContext, "Bluetooth enabling has been cancelled", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }

    fun Context.hasPermission(permissionType: String) : Boolean {
        return ContextCompat.checkSelfPermission(this, permissionType) ==
                PackageManager.PERMISSION_GRANTED
    }

    override fun onBackPressed() {
        val drawer_layout = findViewById<DrawerLayout>(R.id.drawer_layout)

        if (drawer_layout.isDrawerOpen(GravityCompat.START)) {
            drawer_layout.closeDrawer(GravityCompat.START)
        } else {
            super.onBackPressed()
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                val intent = Intent(this, SettingsActivity::class.java)
                startActivity(intent)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onNavigationItemSelected(item: MenuItem): Boolean {
        val drawer_layout = findViewById<DrawerLayout>(R.id.drawer_layout)

        //create a transaction
        val transaction = supportFragmentManager.beginTransaction()
        // handler navigation view item clicks
        when (item.itemId) {
            R.id.nav_autonomous -> {
                // Create a new fragment of the appropriate type
                val fragment = AutoFragment()
                transaction.replace(R.id.fragmentHolder, fragment)
                m_fragment = "a"

                //send message to ESP
                val message = ByteArray(2)
                message[0] = 0.toByte()
                message[1] = 0.toByte()
                m_bluetoothSocket?.outputStream?.write(message)

            }
            R.id.nav_manual -> {
                // Create a new fragment of the appropriate type
                val fragment = ManualFragment()
                transaction.replace(R.id.fragmentHolder, fragment)
                m_fragment = "m"

                //send message to ESP
                val message = ByteArray(2)
                message[0] = 0.toByte()
                message[1] = 1.toByte()
                m_bluetoothSocket?.outputStream?.write(message)

            }
        }

        // remember which menu option was chosen
        transaction.addToBackStack(null)
        transaction.commit()

        if (m_bluetoothSocket == null) {
            Connect(this).execute()
        }

        drawer_layout.closeDrawer(GravityCompat.START)
        return true
    }

    private fun convertToProtocol(n: Int, t: Int, fahrenheit : Boolean): ByteArray {
        var c : Int = t
        //convert temp to celsius
        if(fahrenheit) { c = (((t - 32)*5)/9) }

        //convert temp to 16 bit binary string
        var binary = Integer.toBinaryString(c)
        if (binary.length < 16) {
            val length = binary.length
            val zeros = 16-length
            repeat(zeros) {
                binary = "0$binary"
            }
            Log.i("temperature", ""+binary)
        }

        //assemble the message to send
        val message = ByteArray(3)
        // code for what is being sent(chamber temp, cook temp, ...)
        message[0] = n.toByte()
        // temperature second 8 bits
        message[1] = binary.substring(8,16).toUByte(2).toByte()
        // temperature first 8 bits
        message[2] = binary.substring(0,8).toUByte(2).toByte()
        return message
    }

    fun sendChamberTemperature(v: View) {
        val chamberTemp = findViewById<EditText>(R.id.editChamberTemp)
        val chamberTemperature: Int = chamberTemp.text.toString().toInt()
        if(fahrenheit) {
            userChamberTemp = (((chamberTemperature - 32)*5)/9).toFloat()
        } else {
            userChamberTemp = chamberTemperature.toFloat()
        }
        var message : ByteArray? = null

        if(m_bluetoothSocket == null) {
            Toast.makeText(
                applicationContext,
                "Chamber Temperature not sent, regain connection to IoT Pitmaster",
                Toast.LENGTH_LONG
            )
        } else {
            if (fahrenheit && chamberTemperature > 600) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be below 600°F",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (fahrenheit && chamberTemperature < 100) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be above 100°F",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (!fahrenheit && chamberTemperature > 315) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be below 315°C",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (!fahrenheit && chamberTemperature < 37) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be above 37°C",
                    Toast.LENGTH_SHORT
                ).show()
            } else {
                //send to MCU
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature sent to smoker, monitor temperature below",
                    Toast.LENGTH_SHORT
                ).show()
                //convert to agreed upon coded message to send to MCU
                message = if (fahrenheit) {
                    convertToProtocol(1, chamberTemperature, true)
                } else {
                    convertToProtocol(1, chamberTemperature, false)
                }
                Log.i("Temp", "" + message.toString())
                m_bluetoothSocket?.outputStream?.write(message)
            }
        }
    }

    fun sendCookTemperature(v: View) {
        val cookTemp = findViewById<EditText>(R.id.editCookTemp)
        val cookTemperature: Int = cookTemp.text.toString().toInt()

        if(fahrenheit) {
            userCookTemp = (((cookTemperature - 32)*5)/9).toFloat()
        } else {
            userCookTemp = cookTemperature.toFloat()
        }
        var message : ByteArray? = null


        if(m_bluetoothSocket == null) {
            Toast.makeText(
                applicationContext,
                "Cook Temperature not sent, regain connection to IoT Pitmaster",
                Toast.LENGTH_LONG
            )
        } else {
            if (fahrenheit && cookTemperature > 600) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be below 600°F",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (fahrenheit && cookTemperature < 100) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be above 100°F",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (!fahrenheit && cookTemperature > 315) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be below 315°C",
                    Toast.LENGTH_SHORT
                ).show()
            } else if (!fahrenheit && cookTemperature < 37) {
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature must be above 37°C",
                    Toast.LENGTH_SHORT
                ).show()
            } else {
                //send to MCU
                Toast.makeText(
                    applicationContext,
                    "Desired Temperature sent to smoker, monitor temperature below",
                    Toast.LENGTH_SHORT
                ).show()

                message = if (fahrenheit) {
                    convertToProtocol(2, cookTemperature, true)
                } else {
                    convertToProtocol(2, cookTemperature, false)
                }
                Log.i("Temp", "" + message.toString())
                m_bluetoothSocket?.outputStream?.write(message)
            }
        }
    }

    fun sendDispenseFuel(v: View) {
        if(m_bluetoothSocket == null) {
            Toast.makeText(
                applicationContext,
                "Command not sent, regain connection to IoT Pitmaster",
                Toast.LENGTH_LONG
            )
        } else {
            val message = ByteArray(2)
            message[0] = 5.toByte()
            message[1] = 1.toByte()

            m_bluetoothSocket?.outputStream?.write(message)
        }
    }

    fun sendOpenDamper(v: View) {
        if(m_bluetoothSocket == null) {
            Toast.makeText(
                applicationContext,
                "Command not sent, regain connection to IoT Pitmaster",
                Toast.LENGTH_LONG
            )
        } else {
            val message = ByteArray(2)
            message[0] = 6.toByte()
            message[1] = 1.toByte()

            m_bluetoothSocket?.outputStream?.write(message)
        }
    }

    fun sendCloseDamper(v: View) {
        if(m_bluetoothSocket == null) {
            Toast.makeText(
                applicationContext,
                "Cook Temperature not sent, regain connection to IoT Pitmaster",
                Toast.LENGTH_LONG
            )
        } else {
            val message = ByteArray(2)
            message[0] = 6.toByte()
            message[1] = 0.toByte()

            m_bluetoothSocket?.outputStream?.write(message)
        }
    }

    suspend fun readBluetoothData() {
        val buffer = ByteArray(18)

        // read the input stream every half second while the app is running
        while (true) {
            val bluetoothSocketInputStream = m_bluetoothSocket?.inputStream
            //wait 0.5 second
            try {
                delay(500)
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }

            //read the message
            try {
                if (bluetoothSocketInputStream != null) {
                    bluetoothSocketInputStream.read(buffer)
                    Log.i("debug", "1")
                    decodeAndPost(buffer)
                }
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }
    }

    @SuppressLint("SetTextI18n")
    fun decodeAndPost(readMessage: ByteArray) {
        readMessage.reverse(0,4)
        readMessage.reverse(5,9)
        readMessage.reverse(10,14)
        val byteBuffer = ByteBuffer.wrap(readMessage)


        val chamberTemp: Float = byteBuffer.getFloat(0)
        val cookTempLeft : Float = byteBuffer.getFloat(5)
        val cookTempRight : Float = byteBuffer.getFloat(10)
        val blowFan : Int = byteBuffer.get(15).toInt()
        val hopper : Int = byteBuffer.get(16).toInt()
        val damper : Int = byteBuffer.get(17).toInt()

        //send notification if the temperature is high
        if (chamberTemp > userChamberTemp + 20 || cookTempLeft > userCookTemp + 20 || cookTempRight > userCookTemp + 20) {
            notificationText = "Temperature is running hot, the system is doing what it can to fix"
            notificationId = notificationId++
            notificationTimer(1)
        }

        //send notification when the cook is done
        if (cookTempLeft == userCookTemp || cookTempRight == userCookTemp) {
            notificationText = "Your food is ready!"
            notificationId = notificationId++
            notificationTimer(2)
        }

        var f_chamber : Float = 0F
        var f_cookL : Float = 0F
        var f_cookR : Float = 0F
        if(fahrenheit) {
            f_chamber = ((chamberTemp * 9)/5) + 32
            f_cookL = ((cookTempLeft * 9)/5) + 32
            f_cookR = ((cookTempRight * 9)/5) + 32
        }

        if(m_fragment == "m") {
            runOnUiThread {
                if(fahrenheit) {
                    chamber_temperature_manual.text = "$f_chamber°F"
                    cook_temperature_manual_left.text = "$f_cookL°F"
                    cook_temperature_manual_right.text = "$f_cookR°F"
                } else {
                    chamber_temperature_manual.text = "$chamberTemp°C"
                    cook_temperature_manual_left.text = "$f_cookL°C"
                    cook_temperature_manual_right.text = "$f_cookR°C"
                }

                // hopper
                if (hopper == 1) {
                    hopper_manual.text = getString(R.string.hopper) + ": Dispensing"
                } else {
                    hopper_manual.text = getString(R.string.hopper) + ": Off"
                }

                // damper
                if (damper == 1) {
                    damper_manual.text = getString(R.string.damper) + ": Open"
                } else {
                    damper_manual.text = getString(R.string.damper) + ": Closed"
                }

            }
        } else {
            runOnUiThread {
                if (fahrenheit) {
                    chamber_temp_auto.text =
                        getString(R.string.chamber_temperature) + ": $f_chamber" + "°F"
                    cook_temp_left_auto.text =
                        getString(R.string.cook_temperature_left) + ": $f_cookL" + "°F"
                    cook_temp_right_auto.text =
                        getString(R.string.cook_temperature_right) + ": $f_cookR" + "°F"
                } else {
                    chamber_temp_auto.text =
                        getString(R.string.chamber_temperature) + ": $chamberTemp" + "°C"
                    cook_temp_left_auto.text =
                        getString(R.string.cook_temperature_left) + ": $cookTempLeft" + "°C"
                    cook_temp_right_auto.text =
                        getString(R.string.cook_temperature_right) + ": $cookTempRight" + "°C"
                }
                blow_fan_auto.text = getString(R.string.blow_fan) + ": $blowFan%"

                // hopper
                if (hopper == 1) {
                    hopper_auto.text = getString(R.string.hopper) + ": Dispensing"
                } else {
                    hopper_auto.text = getString(R.string.hopper) + ": Off"
                }

                // damper
                if (damper == 1) {
                    damper_auto.text = getString(R.string.damper) + ": Open"
                } else {
                    damper_auto.text = getString(R.string.damper) + ": Closed"
                }
            }
        }
    }
}