using UnityEngine;
using UnityEngine.InputSystem;

public class FreeCameraController : MonoBehaviour
{
    public float moveSpeed = 20f;
    public float verticalSpeed = 10f;
    public float rotationSpeed = 45f;   // μοίρες ανά δευτερόλεπτο

    void Update()
    {
        if (Keyboard.current == null) return;

        HandleMovementXZ();
        HandleHeight();
        HandleRotation();
    }

    void HandleMovementXZ()
    {
        float moveX = 0f;
        float moveZ = 0f;

        // Βελάκια: Left/Right → X, Up/Down → Z
        if (Keyboard.current.leftArrowKey.isPressed) moveX = -1f;
        if (Keyboard.current.rightArrowKey.isPressed) moveX = 1f;
        if (Keyboard.current.upArrowKey.isPressed) moveZ = 1f;
        if (Keyboard.current.downArrowKey.isPressed) moveZ = -1f;

        Vector3 dir = new Vector3(moveX, 0f, moveZ).normalized;
        transform.Translate(dir * moveSpeed * Time.deltaTime, Space.World);
    }

    void HandleHeight()
    {
        float moveY = 0f;

        // + / - για ύψος (στο keyboard το + είναι shift + =)
        if (Keyboard.current.equalsKey.isPressed) moveY = 1f;   // '+'
        if (Keyboard.current.minusKey.isPressed) moveY = -1f;  // '-'

        Vector3 dir = new Vector3(0f, moveY, 0f);
        transform.Translate(dir * verticalSpeed * Time.deltaTime, Space.World);
    }

    void HandleRotation()
    {
        if (Keyboard.current.rKey.isPressed)
        {
            // Περιστροφή γύρω από τον δικό της Y άξονα
            transform.Rotate(Vector3.up, rotationSpeed * Time.deltaTime, Space.Self);
        }
    }
}
