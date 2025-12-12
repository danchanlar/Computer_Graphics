using UnityEngine;
using UnityEngine.InputSystem;

public class ShooterController : MonoBehaviour
{
    [Header("Movement")]
    public float moveSpeed = 10f;

    [Tooltip("5 διαβαθμίσεις ταχύτητας του shooter")]
    public float[] speedLevels = new float[5] { 5f, 7.5f, 10f, 12.5f, 15f };

    // default: μέση ταχύτητα (index 2 ⇒ 3ο στοιχείο)
    private int currentSpeedIndex = 2;

    [Header("Shooting")]
    public GameObject bulletPrefab;
    public Transform bulletSpawnPoint;
    public float bulletSpeed = 25f;

    private void Start()
    {
        // Αρχικοποίηση moveSpeed από το επιλεγμένο level
        if (speedLevels != null && speedLevels.Length > 0)
        {
            currentSpeedIndex = Mathf.Clamp(currentSpeedIndex, 0, speedLevels.Length - 1);
            moveSpeed = speedLevels[currentSpeedIndex];
        }
    }

    private void Update()
    {
        if (Keyboard.current == null) return;

        HandleSpeedInput();   // νέα συνάρτηση για Z / X / 1–5
        HandleMovement();
        HandleShoot();
    }

    private void HandleSpeedInput()
    {
        // Z → πιο αργά (μείωση level)
        if (Keyboard.current.zKey.wasPressedThisFrame)
        {
            ChangeSpeed(-1);
        }

        // X → πιο γρήγορα (αύξηση level)
        if (Keyboard.current.xKey.wasPressedThisFrame)
        {
            ChangeSpeed(1);
        }

        // Προαιρετικά: 1–5 για κατευθείαν επιλογή level
        if (Keyboard.current.digit1Key.wasPressedThisFrame) SetSpeedLevel(0);
        if (Keyboard.current.digit2Key.wasPressedThisFrame) SetSpeedLevel(1);
        if (Keyboard.current.digit3Key.wasPressedThisFrame) SetSpeedLevel(2);
        if (Keyboard.current.digit4Key.wasPressedThisFrame) SetSpeedLevel(3);
        if (Keyboard.current.digit5Key.wasPressedThisFrame) SetSpeedLevel(4);
    }

    private void ChangeSpeed(int delta)
    {
        if (speedLevels == null || speedLevels.Length == 0) return;

        currentSpeedIndex = Mathf.Clamp(currentSpeedIndex + delta, 0, speedLevels.Length - 1);
        moveSpeed = speedLevels[currentSpeedIndex];

        Debug.Log($"Shooter speed level: {currentSpeedIndex + 1}/{speedLevels.Length}, speed = {moveSpeed}");
    }

    private void SetSpeedLevel(int index)
    {
        if (speedLevels == null || speedLevels.Length == 0) return;

        index = Mathf.Clamp(index, 0, speedLevels.Length - 1);
        currentSpeedIndex = index;
        moveSpeed = speedLevels[currentSpeedIndex];

        Debug.Log($"Shooter speed set to level {currentSpeedIndex + 1}/{speedLevels.Length}, speed = {moveSpeed}");
    }

    private void HandleMovement()
    {
        float move = 0f;

        if (Keyboard.current.jKey.isPressed)
            move = -1f;

        if (Keyboard.current.lKey.isPressed)
            move = 1f;

        Vector3 dir = Vector3.right * move;
        transform.Translate(dir * moveSpeed * Time.deltaTime, Space.World);
    }

    private void HandleShoot()
    {
        if (Keyboard.current.spaceKey.wasPressedThisFrame)
        {
            Shoot();
        }
    }

    private void Shoot()
    {
        if (bulletPrefab == null || bulletSpawnPoint == null)
        {
            Debug.LogWarning("ShooterController: bulletPrefab ή bulletSpawnPoint ΔΕΝ είναι ορισμένα!");
            return;
        }

        GameObject bullet = Instantiate(bulletPrefab, bulletSpawnPoint.position, bulletSpawnPoint.rotation);

        Rigidbody rb = bullet.GetComponent<Rigidbody>();
        if (rb != null)
        {
            // Η σφαίρα πάει προς τα πάνω στον άξονα Y (όπως το έχεις ρυθμίσει)
            rb.linearVelocity = Vector3.up * bulletSpeed;
        }
        else
        {
            Debug.LogWarning("ShooterController: Το bullet prefab δεν έχει Rigidbody!");
        }
    }
}
