$(function() {
    // do not show until getting the lock state from server
    var locked = false;
    const $button = $("#control-btn-lock");
    $button.hide();

    function updateLockState() {
        const lockedStyle = "btn-success";
        const unlockedStyle = "btn-danger";
        if (locked) {
            $button.text("Locked");
            $button.addClass(lockedStyle);
            $button.removeClass(unlockedStyle);
        } else {
            $button.text("Opened");
            $button.addClass(unlockedStyle);
            $button.removeClass(lockedStyle);
        }
    }

    $.ajax({
        url: process.env.APP_API_SERVER + "/api/control/lock/state",
        type: "GET",
    }).done(function(data) {
        locked = data.locked;
        $button.show();
        updateLockState();
    });

    $button.on("click", function() {
        const request = !locked;
        $button.prop("disabled", true);
        $.ajax({
            url: process.env.APP_API_SERVER + "/api/control/lock/set",
            type: "POST",
            contentType: 'application/json',
            data: JSON.stringify({
                "locked": request,
            }),
        }).done(function(data) {
            if (data.succeed) {
                locked = request;
                updateLockState();
            }
        }).always(function() {
            $button.prop("disabled", false);
        });
    });
});