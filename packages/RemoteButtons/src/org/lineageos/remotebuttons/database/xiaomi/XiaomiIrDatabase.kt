/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.database.xiaomi

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import java.io.File
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.lineageos.remotebuttons.database.Brand
import org.lineageos.remotebuttons.database.IrDatabase
import org.lineageos.remotebuttons.ir.CodeSet
import org.lineageos.remotebuttons.ir.DeviceType
import org.lineageos.remotebuttons.ir.IrCode
import org.lineageos.remotebuttons.ir.IrKey
import org.lineageos.remotebuttons.remote.xiaomi.XiaomiRemoteProgrammer

/**
 * IR code sets from the stock Xiaomi database, bundled with the app. Its codes are already in the
 * Xiaomi remote format.
 */
class XiaomiIrDatabase private constructor(context: Context) :
    SQLiteOpenHelper(context, "xiaomi_ir.sqlite", null, 1), IrDatabase {

    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL("CREATE TABLE brand (id INTEGER PRIMARY KEY, name TEXT NOT NULL, initial TEXT NOT NULL)")
        db.execSQL("CREATE TABLE type_brand_rank (type INTEGER, brand INTEGER, rank INTEGER)")
        db.execSQL("CREATE TABLE brand_code_set (type INTEGER, brand INTEGER, code_set INTEGER, rank INTEGER)")
        db.execSQL(
            "CREATE TABLE code_set (id INTEGER PRIMARY KEY, descriptor TEXT NOT NULL, " +
                "power TEXT, vol_up TEXT, vol_down TEXT)",
        )
        db.execSQL("CREATE INDEX brand_code_set_idx ON brand_code_set (type, brand, rank)")
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) = Unit

    override suspend fun brands(type: DeviceType) = queryBrands(
        "SELECT DISTINCT b.id, b.name FROM brand b JOIN brand_code_set r ON r.brand = b.id " +
            "WHERE r.type = ? ORDER BY b.initial, b.name COLLATE NOCASE",
        type.dbId.toString(),
    )

    override suspend fun popularBrands(type: DeviceType) = queryBrands(
        "SELECT b.id, b.name FROM type_brand_rank p JOIN brand b ON b.id = p.brand " +
            "WHERE p.type = ? AND EXISTS (SELECT 1 FROM brand_code_set r WHERE r.type = p.type AND r.brand = p.brand) " +
            "ORDER BY p.rank LIMIT $POPULAR_BRANDS",
        type.dbId.toString(),
    )

    override suspend fun codeSets(type: DeviceType, brand: Brand) = withContext(Dispatchers.IO) {
        readableDatabase.rawQuery(
            "SELECT s.id, s.descriptor, s.power, s.vol_up, s.vol_down FROM brand_code_set r " +
                "JOIN code_set s ON s.id = r.code_set WHERE r.type = ? AND r.brand = ? ORDER BY r.rank",
            arrayOf(type.dbId.toString(), brand.id),
        ).use { c ->
            buildList {
                while (c.moveToNext()) {
                    val keys = listOf(IrKey.POWER to 2, IrKey.VOLUME_UP to 3, IrKey.VOLUME_DOWN to 4)
                        .mapNotNull { (key, column) ->
                            c.getString(column)?.let { key to IrCode.Raw(XiaomiRemoteProgrammer.RAW_FORMAT, it) }
                        }
                        .toMap()
                    add(CodeSet("xiaomi/${c.getInt(0)}", keys, rawParameters = c.getString(1)))
                }
            }
        }
    }

    private suspend fun queryBrands(sql: String, vararg args: String) = withContext(Dispatchers.IO) {
        readableDatabase.rawQuery(sql, args).use { c ->
            buildList { while (c.moveToNext()) add(Brand(c.getInt(0).toString(), c.getString(1))) }
        }
    }

    companion object {
        private const val POPULAR_BRANDS = 10

        @Volatile
        private var instance: XiaomiIrDatabase? = null

        /** Device type ids in the stock database; soundbars and receivers share "PA" codes. */
        private val DeviceType.dbId
            get() = when (this) {
                DeviceType.TV -> 2
                DeviceType.SOUNDBAR, DeviceType.RECEIVER -> 7
            }

        private const val DATABASE_NAME = "xiaomi_ir.sqlite"
        private const val DATABASE_ASSET = "xiaomi_ir.sqlite"

        /** One helper per process, shared by the wizard and TV Settings. */
        fun get(context: Context): XiaomiIrDatabase = instance ?: synchronized(this) {
            instance ?: context.applicationContext.let { appContext ->
                installBundledDatabase(appContext)
                XiaomiIrDatabase(appContext).also { instance = it }
            }
        }

        /** Seeds the app database from its asset on first use, or repairs an empty old database. */
        private fun installBundledDatabase(context: Context) {
            val databaseFile = context.getDatabasePath(DATABASE_NAME)
            val alreadyPopulated = databaseFile.exists() && runCatching {
                SQLiteDatabase.openDatabase(
                    databaseFile.absolutePath,
                    null,
                    SQLiteDatabase.OPEN_READONLY,
                ).use { db ->
                    db.rawQuery("SELECT 1 FROM code_set LIMIT 1", null).use { it.moveToFirst() }
                }
            }.getOrDefault(false)
            if (alreadyPopulated) return

            databaseFile.parentFile?.mkdirs()
            val temporaryFile = File(databaseFile.parentFile, "$DATABASE_NAME.tmp")
            temporaryFile.delete()
            context.assets.open(DATABASE_ASSET).use { input ->
                temporaryFile.outputStream().use { output -> input.copyTo(output) }
            }
            // Replace an empty database left by an earlier app version.
            databaseFile.delete()
            File("${databaseFile.path}-wal").delete()
            File("${databaseFile.path}-shm").delete()
            check(temporaryFile.renameTo(databaseFile)) { "Could not install bundled IR database" }
        }
    }
}
